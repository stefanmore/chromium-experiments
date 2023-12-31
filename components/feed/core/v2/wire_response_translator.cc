// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/feed/core/v2/wire_response_translator.h"
#include "components/supervised_user/core/browser/proto/get_discover_feed_response.pb.h"

namespace feed {

RefreshResponseData WireResponseTranslator::TranslateWireResponse(
    feedwire::Response response,
    StreamModelUpdateRequest::Source source,
    const AccountInfo& account_info,
    base::Time current_time) const {
  return ::feed::TranslateWireResponse(std::move(response), source,
                                       account_info, current_time);
}

RefreshResponseData WireResponseTranslator::TranslateWireResponse(
    supervised_user::GetDiscoverFeedResponse response,
    StreamModelUpdateRequest::Source source,
    const AccountInfo& account_info,
    base::Time current_time) const {
  return ::feed::TranslateWireResponse(std::move(response), source,
                                       account_info, current_time);
}

}  // namespace feed
