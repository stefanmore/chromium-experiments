// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import {getTemplate} from './xf_search_options.html.js';
import {XfSelect} from './xf_select.js';

/**
 * The enumeration of strings used to identify the kind of option that changed.
 */
export enum OptionKind {
  LOCATION = 'location',
  RECENCY = 'recenty',
  FILE_TYPE = 'file-type',
}

/**
 * The interface that specifies the type of the detail field in the custom
 * SEARCH_OPTIONS_CHANGED event.
 */
export interface OptionChange {
  // The kind of option that has changed (LOCATION, RECENCY, FILE_TYPE).
  kind: OptionKind;
  // The current value of the changed option.
  value: string;
}

/**
 * Helper function for constructing a SEARCH_OPTIONS_CHANGED event.
 */
function newSearchOptionsChangedEvent(
    kind: OptionKind, value: string): CustomEvent {
  return new CustomEvent(SEARCH_OPTIONS_CHANGED, {
    bubbles: true,
    composed: true,
    detail: {
      kind: kind,
      value: value,
    },
  });
}

/**
 * Search options is a composite element that allows users to tweak specifics of
 * the file search operation. For example, it presents an UI for selecting the
 * desired file type.
 *
 * It emits the `SEARCH_OPTIONS_CHANGED` event when the options change. The
 * detail filed of the event carries information what option changes and to what
 * value. It is up to the creator of this element to set values that are
 * meaningful. Typical use:
 *
 * const element = document.createElement('xf-search-options');
 * element.setLocationOptions(new Map<string, string>([
 *   ['value-a', 'Text of value A'],
 *   ...
 * ]);
 * element.addEventListener(
 *     SEARCH_OPTIONS_CHANGED, (event) => {
 *       if (event.detail.kind === OptionKind.LOCATION) {
 *         if (event.detail.value === 'value-a') {
 *           // adjust search options, based on value-a being selected.
 *         }
 *       }
 *     });
 */
export class XfSearchOptionsElement extends HTMLElement {
  constructor() {
    super();

    // Create element content.
    const template = document.createElement('template');
    template.innerHTML = getTemplate() as unknown as string;
    const fragment = template.content.cloneNode(true);
    this.attachShadow({mode: 'open'}).appendChild(fragment);
  }

  /**
   * On DOM connected initialize all event listeners. We do not disconnect them,
   * as the only time we get DOM disconnected is when the entire files app is
   * closed.
   */
  connectedCallback(): void {
    this.getLocationSelector().addEventListener(
        XfSelect.events.SELECTION_CHANGED, (event) => {
          this.dispatchEvent(newSearchOptionsChangedEvent(
              OptionKind.LOCATION, event.detail.value));
        });
    this.getRecencySelector().addEventListener(
        XfSelect.events.SELECTION_CHANGED, (event) => {
          this.dispatchEvent(newSearchOptionsChangedEvent(
              OptionKind.RECENCY, event.detail.value));
        });
    this.getFileTypeSelector().addEventListener(
        XfSelect.events.SELECTION_CHANGED, (event) => {
          this.dispatchEvent(newSearchOptionsChangedEvent(
              OptionKind.FILE_TYPE, event.detail.value));
        });
  }

  /**
   * Provides access to the location select element.
   */
  getLocationSelector(): XfSelect {
    return this.getSelectElement_('location-selector');
  }

  /**
   * Provides access to the recency select element.
   */
  getRecencySelector(): XfSelect {
    return this.getSelectElement_('recency-selector');
  }

  /**
   * Provides access to the file type select element.
   */
  getFileTypeSelector(): XfSelect {
    return this.getSelectElement_('type-selector');
  }

  private getSelectElement_(id: string): XfSelect {
    const e = this.shadowRoot!.querySelector<XfSelect>(`#${id}`);
    if (e) {
      return e;
    }
    throw new Error(`Element "${id}" not found`);
  }
}

/**
 * The name of the even generated by this widget.
 */
export const SEARCH_OPTIONS_CHANGED = 'search_options_changed';

/**
 * A custom event that informs the container which option kind change to what
 * value. It is up to the container to interpret these.
 */
export type SearchOptionsChangedEvent = CustomEvent<OptionChange>;

declare global {
  interface HTMLElementEventMap {
    [SEARCH_OPTIONS_CHANGED]: SearchOptionsChangedEvent;
  }

  interface HTMLElementTagNameMap {
    'xf-search-options': XfSearchOptionsElement;
  }
}

customElements.define('xf-search-options', XfSearchOptionsElement);
