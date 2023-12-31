// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/tabs/organization/tab_organization.h"

#include <string>

#include "chrome/browser/ui/tabs/organization/tab_data.h"
#include "chrome/browser/ui/tabs/tab_group.h"
#include "chrome/browser/ui/tabs/tab_group_model.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "third_party/abseil-cpp/absl/types/variant.h"

namespace {
constexpr int kMinValidTabsForOrganizing = 2;
int kNextOrganizationID = 1;
}

TabOrganization::TabOrganization(
    TabDatas tab_datas,
    std::vector<std::u16string> names,
    absl::variant<size_t, std::u16string> current_name,
    absl::optional<UserChoice> choice)
    : names_(names),
      current_name_(current_name),
      choice_(choice),
      organization_id_(kNextOrganizationID) {
  for (auto& tab_data : tab_datas_) {
    tab_data->AddObserver(this);
  }
  kNextOrganizationID++;

  // TabDatas must not be duplicates, immediately destroy TabDatas that are.
  std::vector<content::WebContents*> existing_contents;
  for (auto& tab_data : tab_datas) {
    if (!base::Contains(existing_contents, tab_data->web_contents())) {
      existing_contents.emplace_back(tab_data->web_contents());
      tab_data->AddObserver(this);
      tab_datas_.emplace_back(std::move(tab_data));
    }
  }
}

TabOrganization::~TabOrganization() {
  for (auto& tab_data : tab_datas_) {
    tab_data->RemoveObserver(this);
  }

  for (auto& observer : observers_) {
    observer.OnTabOrganizationDestroyed(organization_id_);
  }
}

const std::u16string TabOrganization::GetDisplayName() const {
  if (absl::holds_alternative<size_t>(current_name())) {
    const size_t index = absl::get<size_t>(current_name());
    CHECK(index < names_.size());
    return names_.at(index);
  } else if (absl::holds_alternative<std::u16string>(current_name())) {
    return absl::get<std::u16string>(current_name());
  }
  return u"";
}

void TabOrganization::AddObserver(TabOrganization::Observer* observer) {
  observers_.AddObserver(observer);
}

void TabOrganization::RemoveObserver(TabOrganization::Observer* observer) {
  observers_.RemoveObserver(observer);
}

bool TabOrganization::IsValidForOrganizing() const {
  if (invalidated_by_tab_change_) {
    return false;
  }

  // there must be at least 2 tabs that are valid for organization.
  int valid_tab_count = 0;
  for (const std::unique_ptr<TabData>& tab_data : tab_datas_) {
    if (tab_data->IsValidForOrganizing()) {
      valid_tab_count++;
      if (valid_tab_count >= kMinValidTabsForOrganizing) {
        return true;
      }
    }
  }
  return false;
}

// TODO(1469128) Add UKM/UMA Logging on user add.
void TabOrganization::AddTabData(std::unique_ptr<TabData> new_tab_data) {
  // Guarantee uniqueness. early return and drop the new tab data if not unique.
  for (std::unique_ptr<TabData>& existing_tab_data : tab_datas_) {
    if (existing_tab_data->web_contents() == new_tab_data->web_contents()) {
      return;
    }
  }

  new_tab_data->AddObserver(this);
  tab_datas_.emplace_back(std::move(new_tab_data));
  NotifyObserversOfUpdate();
}

// TODO(1469128) Add UKM/UMA Logging on user remove.
void TabOrganization::RemoveTabData(TabData::TabID tab_id) {
  TabDatas::iterator position =
      std::find_if(tab_datas_.begin(), tab_datas_.end(),
                   [tab_id](const std::unique_ptr<TabData>& tab_data) {
                     return tab_data->tab_id() == tab_id;
                   });
  CHECK(position != tab_datas_.end());

  user_removed_tab_ids_.push_back(tab_id);
  tab_datas_.erase(position);
  NotifyObserversOfUpdate();
}

void TabOrganization::SetCurrentName(
    absl::variant<size_t, std::u16string> new_current_name) {
  current_name_ = new_current_name;
  NotifyObserversOfUpdate();
}

void TabOrganization::SetFeedback(
    optimization_guide::proto::UserFeedback feedback) {
  feedback_ = feedback;
}

// TODO(1469128) Add UKM/UMA Logging on user accept.
void TabOrganization::Accept() {
  CHECK(!choice_.has_value());
  CHECK(IsValidForOrganizing());
  choice_ = UserChoice::ACCEPTED;

  CHECK(tab_datas_.size() > 0);
  TabStripModel* tab_strip_model = tab_datas_[0]->original_tab_strip_model();
  CHECK(tab_strip_model);
  std::vector<int> valid_indices;
  for (const std::unique_ptr<TabData>& tab_data : tab_datas_) {
    // Individual tabs may become invalid. in those cases, where the tab is
    // invalid but the organization is not, do not include the tab in the
    // organization, but still create the organization.

    const int index =
        tab_strip_model->GetIndexOfWebContents(tab_data->web_contents());
    if (tab_data->IsValidForOrganizing() &&
        !base::Contains(valid_indices, index)) {
      valid_indices.emplace_back(index);
    }
  }
  std::sort(valid_indices.begin(), valid_indices.end());

  tab_groups::TabGroupId group_id =
      tab_strip_model->AddToNewGroup(valid_indices);
  TabGroup* const tab_group =
      tab_strip_model->group_model()->GetTabGroup(group_id);
  tab_groups::TabGroupVisualData new_visual_data(
      GetDisplayName(), tab_group->visual_data()->color());
  tab_group->SetVisualData(std::move(new_visual_data),
                           tab_group->IsCustomized());

  // Move the entire group to the start left of the tabstrip.
  // Iterate through the tabstrip model looking for the first non pinned, non
  // grouped tab. If this group is already in the leftmost position then leave
  // it there. Else move the group at the index of that tab.
  int move_index = tab_strip_model->IndexOfFirstNonPinnedTab();
  while (move_index < tab_strip_model->GetTabCount() &&
         (tab_strip_model->GetTabGroupForTab(move_index).has_value() &&
          tab_strip_model->GetTabGroupForTab(move_index).value() !=
              tab_group->id())) {
    move_index++;
  }
  CHECK(move_index < tab_strip_model->GetTabCount());

  if (tab_strip_model->GetTabGroupForTab(move_index) != tab_group->id()) {
    tab_strip_model->MoveGroupTo(tab_group->id(), move_index);
  }

  NotifyObserversOfUpdate();
}

// TODO(1469128) Add UKM/UMA Logging on user reject.
void TabOrganization::Reject() {
  CHECK(!choice_.has_value());
  choice_ = UserChoice::REJECTED;

  NotifyObserversOfUpdate();
}

void TabOrganization::OnTabDataUpdated(const TabData* tab_data) {
  if (!tab_data->IsValidForOrganizing()) {
    invalidated_by_tab_change_ = true;
  }
  NotifyObserversOfUpdate();
}

void TabOrganization::OnTabDataDestroyed(TabData::TabID tab_id) {
  // Only invalidate if RemoveTabData was not previously called on this tab id.
  // Closure of a tab that is a part of an organization should invalidate it,
  // but removal of the tab from the organization should not.
  if (std::find(user_removed_tab_ids_.begin(), user_removed_tab_ids_.end(),
                tab_id) == user_removed_tab_ids_.end()) {
    invalidated_by_tab_change_ = true;
    NotifyObserversOfUpdate();
  }
}

void TabOrganization::NotifyObserversOfUpdate() {
  for (auto& observer : observers_) {
    observer.OnTabOrganizationUpdated(this);
  }
}
