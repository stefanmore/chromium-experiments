// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview Polymer element for signin fatal error.
 */

import '//resources/js/action_link.js';
import '//resources/polymer/v3_0/iron-icon/iron-icon.js';
import '../../components/oobe_icons.html.js';
import '../../components/common_styles/oobe_common_styles.css.js';
import '../../components/common_styles/oobe_dialog_host_styles.css.js';
import '../../components/dialogs/oobe_adaptive_dialog.js';

import {html, mixinBehaviors, PolymerElement} from '//resources/polymer/v3_0/polymer/polymer_bundled.min.js';

import {LoginScreenBehavior, LoginScreenBehaviorInterface} from '../../components/behaviors/login_screen_behavior.js';
import {OobeDialogHostBehavior} from '../../components/behaviors/oobe_dialog_host_behavior.js';
import {OobeI18nBehavior, OobeI18nBehaviorInterface} from '../../components/behaviors/oobe_i18n_behavior.js';
import {OobeTextButton} from '../../components/buttons/oobe_text_button.js';
import {OOBE_UI_STATE, SCREEN_GAIA_SIGNIN} from '../../components/display_manager_types.js';
import {OobeTypes} from '../../components/oobe_types.js';


/**
 * @constructor
 * @extends {PolymerElement}
 * @implements {LoginScreenBehaviorInterface}
 * @implements {OobeI18nBehaviorInterface}
 */
const SigninFatalErrorBase = mixinBehaviors(
    [OobeI18nBehavior, OobeDialogHostBehavior, LoginScreenBehavior],
    PolymerElement);

/**
 * @typedef {{
 *   actionButton:  OobeTextButton,
 * }}
 */
SigninFatalErrorBase.$;

/**
 * Data that is passed to the screen during onBeforeShow.
 * @typedef {{
 *   errorState: OobeTypes.FatalErrorCode,
 *   errorText: (string|undefined),
 *   keyboardHint: (string|undefined),
 *   details: (string|undefined),
 *   helpLinkText: (string|undefined),
 *   url: (string|undefined),
 * }}
 */
let SigninFatalErrorScreenData;

/**
 * @polymer
 */
class SigninFatalScreen extends SigninFatalErrorBase {
  static get is() {
    return 'signin-fatal-error-element';
  }

  static get template() {
    return html`{__html_template__}`;
  }


  static get properties() {
    return {
      /**
       * Subtitle that will be shown to the user describing the error
       * @private
       */
      errorSubtitle_: {
        type: String,
        computed: 'computeSubtitle_(locale, errorState_, params_)',
      },

      /**
       * Error state from the screen
       * @type {OobeTypes.FatalErrorCode}
       * @private
       */
      errorState_: {
        type: Number,
        value: OobeTypes.FatalErrorCode.UNKNOWN,
      },

      /**
       * Additional information that will be used when creating the subtitle.
       * @type {SigninFatalErrorScreenData}
       * @private
       */
      params_: {
        type: Object,
        value: {},
      },

      /**
       * @type {(string|undefined)}
       * @private
       */
      keyboardHint_: {
        type: String,
      },

      /**
       * @type {(string|undefined)}
       * @private
       */
      details_: {
        type: String,
      },

      /**
       * @type {(string|undefined)}
       * @private
       */
      helpLinkText_: {
        type: String,
      },
    };
  }

  ready() {
    super.ready();
    this.initializeLoginScreen('SignInFatalErrorScreen');
  }

  /** Initial UI State for screen */
  getOobeUIInitialState() {
    return OOBE_UI_STATE.BLOCKING;
  }

  /**
   * Returns the control which should receive initial focus.
   */
  get defaultControl() {
    return this.$.actionButton;
  }

  /**
   * Invoked just before being shown. Contains all the data for the screen.
   * @param {SigninFatalErrorScreenData} data Screen init payload.
   */
  onBeforeShow(data) {
    this.errorState_ = data?.errorState;
    this.params_ = data;
    this.keyboardHint_ = this.params_.keyboardHint;
    this.details_ = this.params_.details;
    this.helpLinkText_ = this.params_.helpLinkText;
  }

  onClick_() {
    this.userActed('screen-dismissed');
  }

  /**
   * Generates the key for the button that is shown to the
   * user based on the error
   * @param {number} error_state
   * @private
   * @suppress {missingProperties} OobeTypes
   */
  computeButtonKey_(error_state) {
    if (this.errorState_ == OobeTypes.FatalErrorCode.INSECURE_CONTENT_BLOCKED) {
      return 'fatalErrorDoneButton';
    }

    return 'fatalErrorTryAgainButton';
  }

  /**
   * Generates the subtitle that is shown to the
   * user based on the error
   * @param {string} locale
   * @param {OobeTypes.FatalErrorCode} error_state
   * @param {string} params
   * @private
   */
  computeSubtitle_(locale, error_state, params) {
    switch (this.errorState_) {
      case OobeTypes.FatalErrorCode.SCRAPED_PASSWORD_VERIFICATION_FAILURE:
        return this.i18n('fatalErrorMessageVerificationFailed');
      case OobeTypes.FatalErrorCode.MISSING_GAIA_INFO:
        return this.i18n('fatalErrorMessageNoAccountDetails');
      case OobeTypes.FatalErrorCode.INSECURE_CONTENT_BLOCKED:
        /**
         * @suppress {checkTypes} We know that url is already a valid string.
         * @type {string}
         */
        const url = this.params_?.url;
        return this.i18n('fatalErrorMessageInsecureURL', url);
      case OobeTypes.FatalErrorCode.CUSTOM:
        return this.params_.errorText;
      case OobeTypes.FatalErrorCode.UNKNOWN:
        return '';
    }
  }

  onHelpLinkClicked_() {
    this.userActed('learn-more');
  }
}

customElements.define(SigninFatalScreen.is, SigninFatalScreen);
