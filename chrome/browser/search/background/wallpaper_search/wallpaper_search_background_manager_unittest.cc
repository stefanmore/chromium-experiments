// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/search/background/wallpaper_search/wallpaper_search_background_manager.h"

#include <vector>

#include "base/files/file_util.h"
#include "base/path_service.h"
#include "base/token.h"
#include "build/build_config.h"
#include "chrome/browser/search/background/ntp_custom_background_service_factory.h"
#include "chrome/common/pref_names.h"
#include "chrome/common/url_constants.h"
#include "chrome/test/base/testing_profile.h"
#include "components/prefs/pref_service.h"
#include "components/sync_preferences/testing_pref_service_syncable.h"
#include "content/public/test/browser_task_environment.h"
#include "content/public/test/test_utils.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/gfx/image/image.h"
#include "url/gurl.h"

namespace {

using testing::Return;
using testing::SaveArg;

class MockNtpCustomBackgroundService : public NtpCustomBackgroundService {
 public:
  explicit MockNtpCustomBackgroundService(Profile* profile)
      : NtpCustomBackgroundService(profile) {}
  MOCK_METHOD(absl::optional<CustomBackground>, GetCustomBackground, ());
  MOCK_METHOD0(IsCustomBackgroundDisabledByPolicy, bool());
  MOCK_METHOD1(SetBackgroundToLocalResourceWithId, void(const base::Token&));
  MOCK_METHOD1(UpdateCustomLocalBackgroundColorAsync, void(const gfx::Image&));
};

std::unique_ptr<TestingProfile> MakeTestingProfile() {
  TestingProfile::Builder profile_builder;
  profile_builder.AddTestingFactory(
      NtpCustomBackgroundServiceFactory::GetInstance(),
      base::BindRepeating([](content::BrowserContext* context)
                              -> std::unique_ptr<KeyedService> {
        return std::make_unique<
            testing::NiceMock<MockNtpCustomBackgroundService>>(
            Profile::FromBrowserContext(context));
      }));
  auto profile = profile_builder.Build();
  return profile;
}

}  // namespace

class WallpaperSearchBackgroundManagerTest : public testing::Test {
 public:
  WallpaperSearchBackgroundManagerTest()
      : profile_(MakeTestingProfile()),
        mock_ntp_custom_background_service_(
            static_cast<MockNtpCustomBackgroundService*>(
                NtpCustomBackgroundServiceFactory::GetForProfile(
                    profile_.get()))),
        pref_service_(profile_->GetPrefs()) {}

  void SetUp() override {
    // Register |WallpaperSearchBackgroundManager| prefs. Enabling flag here
    // so that it should be registered by |NtpCustomBackgroundService|
    // doesn't work. NtpCustomBackgroundService::RegisterProfilePrefs must be
    // called before this.
    WallpaperSearchBackgroundManager::RegisterProfilePrefs(
        profile_->GetTestingPrefService()->registry());
    wallpaper_search_background_manager_ =
        std::make_unique<WallpaperSearchBackgroundManager>(profile_.get());
  }

  base::FilePath GetFilePathForBackground(base::Token id) {
    return profile_->GetPath().AppendASCII(
        id.ToString() + chrome::kChromeUIUntrustedNewTabPageBackgroundFilename);
  }

  MockNtpCustomBackgroundService& mock_ntp_custom_background_service() {
    return *mock_ntp_custom_background_service_;
  }
  PrefService& pref_service() { return *pref_service_; }
  TestingProfile& profile() { return *profile_; }
  content::BrowserTaskEnvironment& task_environment() {
    return task_environment_;
  }
  WallpaperSearchBackgroundManager& wallpaper_search_background_manager() {
    return *wallpaper_search_background_manager_;
  }

 private:
  // NOTE: The initialization order of these members matters.
  content::BrowserTaskEnvironment task_environment_;
  std::unique_ptr<TestingProfile> profile_;
  raw_ptr<MockNtpCustomBackgroundService> mock_ntp_custom_background_service_;
  raw_ptr<PrefService> pref_service_;
  std::unique_ptr<WallpaperSearchBackgroundManager>
      wallpaper_search_background_manager_;
};

TEST_F(WallpaperSearchBackgroundManagerTest, GetHistory) {
  // Fill history pref.
  base::Value::List history = base::Value::List();
  std::vector<base::Token> tokens;
  for (int i = 0; i < 3; i++) {
    base::Token temp_token = base::Token::CreateRandom();
    tokens.push_back(temp_token);
    history.Append(temp_token.ToString());
  }
  pref_service().SetList(prefs::kNtpWallpaperSearchHistory, std::move(history));

  auto result = wallpaper_search_background_manager().GetHistory();
  EXPECT_EQ(result, tokens);
}

TEST_F(WallpaperSearchBackgroundManagerTest, SetHistoryImage) {
  gfx::Image image_arg;
  base::Token token_arg;
  ON_CALL(mock_ntp_custom_background_service(),
          IsCustomBackgroundDisabledByPolicy)
      .WillByDefault(testing::Return(false));
  EXPECT_CALL(mock_ntp_custom_background_service(),
              SetBackgroundToLocalResourceWithId)
      .WillOnce(SaveArg<0>(&token_arg));
  EXPECT_CALL(mock_ntp_custom_background_service(),
              UpdateCustomLocalBackgroundColorAsync)
      .WillOnce(SaveArg<0>(&image_arg));
  SkBitmap bitmap;
  bitmap.allocN32Pixels(32, 32);
  bitmap.eraseColor(SK_ColorRED);

  const gfx::Image& image = gfx::Image::CreateFrom1xBitmap(bitmap);
  const base::Token& token = base::Token::CreateRandom();
  wallpaper_search_background_manager().SelectHistoryImage(token, image);
  task_environment().RunUntilIdle();

  // Check that the args were passed to |NtpCustomBackgroundService|.
  EXPECT_EQ(token_arg, token);
  EXPECT_EQ(image, image_arg);
}

TEST_F(WallpaperSearchBackgroundManagerTest, SetLocalBackgroundImage) {
  gfx::Image image_arg;
  base::Token token_arg;
  ON_CALL(mock_ntp_custom_background_service(),
          IsCustomBackgroundDisabledByPolicy)
      .WillByDefault(testing::Return(false));
  EXPECT_CALL(mock_ntp_custom_background_service(),
              SetBackgroundToLocalResourceWithId)
      .WillOnce(SaveArg<0>(&token_arg));
  EXPECT_CALL(mock_ntp_custom_background_service(),
              UpdateCustomLocalBackgroundColorAsync)
      .WillOnce(SaveArg<0>(&image_arg));

  SkBitmap bitmap;
  bitmap.allocN32Pixels(32, 32);
  bitmap.eraseColor(SK_ColorRED);

  base::Token token = base::Token::CreateRandom();
  wallpaper_search_background_manager().SelectLocalBackgroundImage(token,
                                                                   bitmap);
  task_environment().RunUntilIdle();

  // Check that image file was created.
  EXPECT_TRUE(base::PathExists(GetFilePathForBackground(token)));

  // Check that the args were passed to |NtpCustomBackgroundService|.
  EXPECT_EQ(token_arg.high(), token.high());
  EXPECT_EQ(token_arg.low(), token.low());
  EXPECT_EQ(SK_ColorRED, image_arg.ToSkBitmap()->getColor(0, 0));
}

// If the currently set wallpaper search image is set again, do not pass it
// through to SetBackgroundToLocalResourceWithId(). Otherwise, its image file
// could be deleted.
TEST_F(WallpaperSearchBackgroundManagerTest,
       SetLocalBackgroundImage_DoNotReSetSameImage) {
  gfx::Image image_arg;
  ON_CALL(mock_ntp_custom_background_service(),
          IsCustomBackgroundDisabledByPolicy)
      .WillByDefault(testing::Return(false));
  EXPECT_CALL(mock_ntp_custom_background_service(),
              SetBackgroundToLocalResourceWithId)
      .Times(0);
  EXPECT_CALL(mock_ntp_custom_background_service(),
              UpdateCustomLocalBackgroundColorAsync)
      .WillOnce(SaveArg<0>(&image_arg));

  SkBitmap bitmap;
  bitmap.allocN32Pixels(32, 32);
  bitmap.eraseColor(SK_ColorRED);

  base::Token token = base::Token::CreateRandom();
  CustomBackground custom_background;
  custom_background.local_background_id = token;
  ON_CALL(mock_ntp_custom_background_service(), GetCustomBackground())
      .WillByDefault(Return(absl::make_optional(custom_background)));
  wallpaper_search_background_manager().SelectLocalBackgroundImage(token,
                                                                   bitmap);

  task_environment().RunUntilIdle();

  // Check that the args were passed to |NtpCustomBackgroundService|.
  EXPECT_EQ(SK_ColorRED, image_arg.ToSkBitmap()->getColor(0, 0));
}

TEST_F(WallpaperSearchBackgroundManagerTest, SaveCurrentBackgroundToHistory) {
  base::Token token = base::Token::CreateRandom();
  CustomBackground custom_background;
  custom_background.local_background_id = token;
  ON_CALL(mock_ntp_custom_background_service(), GetCustomBackground())
      .WillByDefault(Return(absl::make_optional(custom_background)));

  wallpaper_search_background_manager().SaveCurrentBackgroundToHistory();
  task_environment().RunUntilIdle();

  const base::Value::List& history =
      pref_service().GetList(prefs::kNtpWallpaperSearchHistory);
  ASSERT_EQ(history.size(), 1u);
  ASSERT_TRUE(history.front().is_string());
  EXPECT_EQ(token.ToString(), history.front().GetString());
}

// Test that the last history entry is deleted when a new entry is added,
// if the list is full.
TEST_F(WallpaperSearchBackgroundManagerTest,
       SaveCurrentBackgroundToHistory_FullHistory) {
  // Fill history and create files.
  base::Value::List history = base::Value::List();
  std::vector<base::Token> tokens;
  for (int i = 0; i < 6; ++i) {
    base::Token temp_token = base::Token::CreateRandom();
    tokens.push_back(temp_token);
    history.Append(temp_token.ToString());
    base::WriteFile(GetFilePathForBackground(temp_token), "hi");
  }
  pref_service().SetList(prefs::kNtpWallpaperSearchHistory, std::move(history));

  // Set new token theme to be saved to history.
  base::Token theme_token = base::Token::CreateRandom();
  CustomBackground custom_background;
  custom_background.local_background_id = theme_token;
  ON_CALL(mock_ntp_custom_background_service(), GetCustomBackground())
      .WillByDefault(Return(absl::make_optional(custom_background)));

  wallpaper_search_background_manager().SaveCurrentBackgroundToHistory();
  task_environment().RunUntilIdle();

  // Check that the history is still 6 long and the first entry is the new one.
  const base::Value::List& new_history =
      pref_service().GetList(prefs::kNtpWallpaperSearchHistory);
  ASSERT_EQ(new_history.size(), 6u);
  ASSERT_TRUE(new_history.front().is_string());
  EXPECT_EQ(theme_token.ToString(), new_history.front().GetString());

  // Check that the file for deleted history entry has been deleted and the
  // rest are still there.
  for (int i = 0; i < 5; ++i) {
    EXPECT_TRUE(base::PathExists(GetFilePathForBackground(tokens[i])));
  }
  EXPECT_FALSE(base::PathExists(GetFilePathForBackground(tokens[5])));
}

// Test when the new history entry is already in history. Nothing should be
// deleted and the matching history entry should move to the front.
TEST_F(WallpaperSearchBackgroundManagerTest,
       SaveCurrentBackgroundToHistory_AlreadyInHistory) {
  // Fill history and create files.
  base::Value::List history = base::Value::List();
  std::vector<base::Token> tokens;
  for (int i = 0; i < 6; ++i) {
    base::Token temp_token = base::Token::CreateRandom();
    tokens.push_back(temp_token);
    history.Append(temp_token.ToString());
    base::WriteFile(GetFilePathForBackground(temp_token), "hi");
  }
  pref_service().SetList(prefs::kNtpWallpaperSearchHistory, std::move(history));

  // Set new token theme to be saved to history.
  base::Token theme_token = tokens[2];
  CustomBackground custom_background;
  custom_background.local_background_id = theme_token;
  ON_CALL(mock_ntp_custom_background_service(), GetCustomBackground())
      .WillByDefault(Return(absl::make_optional(custom_background)));

  wallpaper_search_background_manager().SaveCurrentBackgroundToHistory();
  task_environment().RunUntilIdle();

  // Check that the history is still 6 long and in the correct order.
  // |theme_token| should be at the front and everything else
  // is the same relative order as before.
  const base::Value::List& new_history =
      pref_service().GetList(prefs::kNtpWallpaperSearchHistory);
  ASSERT_EQ(new_history.size(), 6u);
  ASSERT_TRUE(new_history.front().is_string());
  EXPECT_EQ(theme_token.ToString(), new_history.front().GetString());
  bool before_entry_pos = true;
  for (int i = 1; i < 6; ++i) {
    // If we haven't hit where |theme_token| used to be in the history,
    // the entry we are looking at will be one index back in |tokens| vs
    // |new_history|.
    ASSERT_TRUE(new_history[i].is_string());
    EXPECT_EQ(new_history[i].GetString(),
              tokens[before_entry_pos ? i - 1 : i].ToString());

    if (tokens[i].ToString() == theme_token.ToString()) {
      before_entry_pos = false;
    }
  }

  // Check that no files were deleted.
  for (int i = 0; i < 6; ++i) {
    EXPECT_TRUE(base::PathExists(GetFilePathForBackground(tokens[i])));
  }
}

// Test when the new history entry is the last entry in history. Nothing should
// be deleted and the matching history entry should move to the front.
TEST_F(WallpaperSearchBackgroundManagerTest,
       SaveCurrentBackgroundToHistory_LastInHistory) {
  // Fill history and create files.
  base::Value::List history = base::Value::List();
  std::vector<base::Token> tokens;
  for (int i = 0; i < 6; ++i) {
    base::Token temp_token = base::Token::CreateRandom();
    tokens.push_back(temp_token);
    history.Append(temp_token.ToString());
    base::WriteFile(GetFilePathForBackground(temp_token), "hi");
  }
  pref_service().SetList(prefs::kNtpWallpaperSearchHistory, std::move(history));

  // Set new token theme to be saved to history.
  base::Token theme_token = tokens[5];
  CustomBackground custom_background;
  custom_background.local_background_id = theme_token;
  ON_CALL(mock_ntp_custom_background_service(), GetCustomBackground())
      .WillByDefault(Return(absl::make_optional(custom_background)));

  wallpaper_search_background_manager().SaveCurrentBackgroundToHistory();
  task_environment().RunUntilIdle();

  // Check that the history is still 6 long and in the correct order.
  // |theme_token| should be at the front and everything else
  // is the same relative order as before.
  const base::Value::List& new_history =
      pref_service().GetList(prefs::kNtpWallpaperSearchHistory);
  ASSERT_EQ(new_history.size(), 6u);
  ASSERT_TRUE(new_history.front().is_string());
  EXPECT_EQ(theme_token.ToString(), new_history.front().GetString());
  for (int i = 0; i < 5; ++i) {
    ASSERT_TRUE(new_history[i + 1].is_string());
    EXPECT_EQ(new_history[i + 1].GetString(), tokens[i].ToString());
  }

  // Check that no files were deleted.
  for (int i = 0; i < 6; ++i) {
    EXPECT_TRUE(base::PathExists(GetFilePathForBackground(tokens[i])));
  }
}

// Test that a wallpaper search background is removed if it is not in history
TEST_F(WallpaperSearchBackgroundManagerTest,
       RemoveWallpaperSearchBackground_NotHistory) {
  // Set theme to prefs and create file for it.
  base::Token token = base::Token::CreateRandom();
  pref_service().SetString(prefs::kNtpCustomBackgroundLocalToDeviceId,
                           token.ToString());
  base::WriteFile(GetFilePathForBackground(token), "hi");
  EXPECT_TRUE(base::PathExists(GetFilePathForBackground(token)));

  // Clear wallpaper search theme resource.
  WallpaperSearchBackgroundManager::RemoveWallpaperSearchBackground(&profile());
  task_environment().RunUntilIdle();

  // The theme file created above should now be gone.
  EXPECT_FALSE(base::PathExists(GetFilePathForBackground(token)));
}

// Test that a wallpaper search background is not removed if it is in history
TEST_F(WallpaperSearchBackgroundManagerTest,
       RemoveWallpaperSearchBackground_History) {
  // Set theme to prefs, add it to history, and write it to file.
  base::Token token = base::Token::CreateRandom();
  base::Value::List history = base::Value::List().Append(token.ToString());
  pref_service().SetList(prefs::kNtpWallpaperSearchHistory, std::move(history));
  base::WriteFile(GetFilePathForBackground(token), "hi");

  // Set theme to prefs using a token already in history and check that its file
  // is there.
  pref_service().SetString(prefs::kNtpCustomBackgroundLocalToDeviceId,
                           token.ToString());
  EXPECT_TRUE(base::PathExists(GetFilePathForBackground(token)));

  // Clear wallpaper search theme resource.
  WallpaperSearchBackgroundManager::RemoveWallpaperSearchBackground(&profile());
  task_environment().RunUntilIdle();

  // The theme file created above should still be there.
  EXPECT_TRUE(base::PathExists(GetFilePathForBackground(token)));
}
