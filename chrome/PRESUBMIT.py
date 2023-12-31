# Copyright 2011 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Presubmit script for changes affecting chrome/

See http://dev.chromium.org/developers/how-tos/depottools/presubmit-scripts
for more details about the presubmit API built into depot_tools.
"""


import re

INCLUDE_CPP_FILES_ONLY = (
  r'.*\.(cc|h)$',
)

INCLUDE_SOURCE_FILES_ONLY = (
  r'.*\.(c|cc|cpp|h|m|mm)$',
)

EXCLUDE = (
  # Objective C confuses everything.
  r'.*cocoa.*',
  r'.*_mac\.(cc|h)$',
  r'.*_mac_.*',
  # All the messages files do weird multiple include trickery
  r'.*_messages.*\.h$',
  # Autogenerated window resources files are off limits
  r'.*resource.h$',
  # Header trickery
  r'.*-inl\.h$',
  # Has safe printf usage that cpplint complains about
  r'safe_browsing_util\.cc$',
)

def _CheckChangeLintsClean(input_api, output_api):
  """Makes sure that the chrome/ code is cpplint clean."""
  files_to_skip = input_api.DEFAULT_FILES_TO_SKIP + EXCLUDE
  sources = lambda x: input_api.FilterSourceFile(
    x, files_to_check=INCLUDE_CPP_FILES_ONLY, files_to_skip=files_to_skip)
  return input_api.canned_checks.CheckChangeLintsClean(
      input_api, output_api, sources)


def _CheckNoContentUnitTestsInChrome(input_api, output_api):
  """Makes sure that no unit tests from content/ are included in unit_tests."""
  problems = []
  for f in input_api.AffectedFiles():
    if not f.LocalPath().endswith('BUILD.gn'):
      continue

    for line_num, line in f.ChangedContents():
      m = re.search(r"'(.*\/content\/.*unittest.*)'", line)
      if m:
        problems.append(m.group(1))

  if not problems:
    return []
  return [output_api.PresubmitPromptWarning(
      'Unit tests located in content/ should be added to the ' +
      'content_unittests target.',
      items=problems)]


def _CheckNoIsAppleBuildFlagsInChromeFile(input_api, f):
  """Check for IS_APPLE in a given file in chrome/."""
  preprocessor_statement = input_api.re.compile(r'^\s*#')
  apple_buildflag = input_api.re.compile(r'BUILDFLAG\(IS_APPLE\)')
  results = []
  for lnum, line in f.ChangedContents():
    if preprocessor_statement.search(line) and apple_buildflag.search(line):
      results.append('    %s:%d' % (f.LocalPath(), lnum))

  return results


def _CheckNoIsAppleBuildFlagsInChrome(input_api, output_api):
  """Check for IS_APPLE which isn't used in chrome/."""
  apple_buildflags = []
  def SourceFilter(affected_file):
    return input_api.FilterSourceFile(affected_file, INCLUDE_SOURCE_FILES_ONLY,
                                      input_api.DEFAULT_FILES_TO_SKIP)
  for f in input_api.AffectedSourceFiles(SourceFilter):
    apple_buildflags.extend(_CheckNoIsAppleBuildFlagsInChromeFile(input_api, f))

  if not apple_buildflags:
    return []

  return [output_api.PresubmitError(
      'IS_APPLE is not used in chrome/ but found in:\n', apple_buildflags)]


def _CheckNoIsIOSBuildFlagsInChromeFile(input_api, f):
  """Check for IS_IOS in a given file in chrome/."""
  preprocessor_statement = input_api.re.compile(r'^\s*#')
  ios_buildflag = input_api.re.compile(r'BUILDFLAG\(IS_IOS\)')
  results = []
  for lnum, line in f.ChangedContents():
    if preprocessor_statement.search(line) and ios_buildflag.search(line):
      results.append('    %s:%d' % (f.LocalPath(), lnum))

  return results


def _CheckNoIsIOSBuildFlagsInChrome(input_api, output_api):
  """Check for IS_IOS which isn't used in chrome/."""
  ios_buildflags = []
  def SourceFilter(affected_file):
    return input_api.FilterSourceFile(affected_file, INCLUDE_SOURCE_FILES_ONLY,
                                      input_api.DEFAULT_FILES_TO_SKIP)
  for f in input_api.AffectedSourceFiles(SourceFilter):
    ios_buildflags.extend(_CheckNoIsIOSBuildFlagsInChromeFile(input_api, f))

  if not ios_buildflags:
    return []

  return [output_api.PresubmitError(
      'IS_IOS is not used in chrome/ but found in:\n', ios_buildflags)]


def _CheckBreakingInstallerVersionBumpNeeded(input_api, output_api):
  files = []
  breaking_version_installer_updated = False

  def _FilterFile(affected_file):
    return input_api.FilterSourceFile(
        affected_file,
        files_to_check=input_api.DEFAULT_FILES_TO_CHECK + (r'.*\.release',))
  for f in input_api.AffectedSourceFiles(_FilterFile):
    # Normalize the local path to Linux-style path separators so that the path
    # comparisons work on Windows as well.
    local_path = f.LocalPath().replace('\\', '/')
    breaking_version_installer_updated |= (local_path ==
    'chrome/installer/setup/last_breaking_installer_version.cc')
    if (local_path == 'chrome/installer/mini_installer/chrome.release' or
        local_path.startswith('chrome/test/mini_installer')):
      files.append(local_path)

  if files and not breaking_version_installer_updated:
    return [output_api.PresubmitPromptWarning('''
Update chrome/installer/setup/last_breaking_installer_version.cc if the changes
found in the following files might break make downgrades not possible beyond
this browser's version.''', items=files)]

  if not files and breaking_version_installer_updated:
    return [output_api.PresubmitPromptWarning('''
No installer breaking changes detected but
chrome/installer/setup/last_breaking_installer_version.cc was updated. Please
update chrome/installer/PRESUBMIT.py if more files need to be watched for
breaking installer changes.''')]

  return []


def _CommonChecks(input_api, output_api):
  """Checks common to both upload and commit."""
  results = []
  results.extend(_CheckNoContentUnitTestsInChrome(input_api, output_api))
  results.extend(_CheckNoIsAppleBuildFlagsInChrome(input_api, output_api))
  results.extend(_CheckNoIsIOSBuildFlagsInChrome(input_api, output_api))
  results.extend(_CheckBreakingInstallerVersionBumpNeeded(input_api,
                 output_api))
  return results


def CheckChangeOnUpload(input_api, output_api):
  results = []
  results.extend(_CommonChecks(input_api, output_api))
  results.extend(_CheckChangeLintsClean(input_api, output_api))
  return results


def CheckChangeOnCommit(input_api, output_api):
  results = []
  results.extend(_CommonChecks(input_api, output_api))
  return results
