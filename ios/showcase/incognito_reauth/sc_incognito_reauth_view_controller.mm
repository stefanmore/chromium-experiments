// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/showcase/incognito_reauth/sc_incognito_reauth_view_controller.h"

#import "ios/chrome/browser/ui/incognito_reauth/incognito_reauth_view.h"

@implementation SCIncognitoReauthViewController

- (void)loadView {
  self.view = [[IncognitoReauthView alloc] init];
}

@end
