import sys
import os

# When loaded by xteink-plugins framework, sys.path is configured.
# When loaded locally or during tests, ensure parent directories are accessible.
current_dir = os.path.dirname(os.path.abspath(__file__))
parent_dir = os.path.dirname(current_dir)
grandparent_dir = os.path.dirname(parent_dir)
for p in [current_dir, parent_dir, grandparent_dir]:
    if p not in sys.path:
        sys.path.insert(0, p)

try:
    from framework.manifest import (
        PluginManifest, SourceFile, Include, SettingsField, PluginsTabEntry, SettingActionEnumValue,
    )
except ImportError:
    # Fallback to local import if needed
    try:
        from xteink_plugins.framework.manifest import (
            PluginManifest, SourceFile, Include, SettingsField, PluginsTabEntry, SettingActionEnumValue,
        )
    except ImportError:
        pass

try:
    from .patch import patch_crosspoint_reader
except ImportError:
    from patch import patch_crosspoint_reader


def _post_patch(ctx):
    patch_crosspoint_reader(ctx.repo_dir)


def get_manifest(ctx):
    return PluginManifest(
        name="vocabbuilder",
        pretty_name="Vocabulary Builder",
        source_files=[
            SourceFile("VocabBuilderPlugin.h", "src/activities/settings"),
            SourceFile("VocabBuilderPlugin.cpp", "src/activities/settings"),
            SourceFile("VocabBuilderActivity.h", "src/activities/settings"),
            SourceFile("VocabBuilderActivity.cpp", "src/activities/settings"),
            SourceFile("VocabBuilderStore.h", "src/vocabulary"),
            SourceFile("VocabBuilderStore.cpp", "src/vocabulary"),
            SourceFile("PluginListDraw.h", "src/activities/settings"),
        ],
        includes=[
            Include("activities/settings/VocabBuilderPlugin.h", "cross_point_settings_h"),
            Include("activities/settings/VocabBuilderPlugin.h", "settings_activity_cpp"),
            Include("activities/settings/VocabBuilderActivity.h", "settings_activity_cpp"),
        ],
        settings_fields=[
            SettingsField("uint8_t vocabDefaultAction = 0;"),
            SettingsField("uint8_t vocabLowercase = 1;"),
            SettingsField("uint8_t vocabAutoSaveLookup = 0;"),
            SettingsField("char    vocabFilePath[64] = \"/vocabulary_builder.txt\";"),
        ],
        setting_actions=[
            SettingActionEnumValue("VocabBuilder"),
        ],
        plugins_tab_entries=[
            PluginsTabEntry(
                label="Vocabulary Builder",
                kind="action",
                action_name="VocabBuilder",
                action_value_text="Open",
                activity_launch_expr="std::make_unique<VocabBuilderActivity>(renderer, mappedInput)",
            ),
            PluginsTabEntry(
                label="Confirm Action",
                kind="enum",
                key="vocabDefaultAction",
                option_labels=["Clip", "Save to Vocab"],
                value_text_expr="VocabBuilderPlugin::defaultActionName(static_cast<VocabDefaultAction>(value))",
            ),
            PluginsTabEntry(
                label="Lowercase Words",
                kind="enum",
                key="vocabLowercase",
                option_labels=["Disabled", "Enabled"],
                value_text_expr="VocabBuilderPlugin::toggleName(value)",
            ),
            PluginsTabEntry(
                label="Auto-Save Lookups",
                kind="enum",
                key="vocabAutoSaveLookup",
                option_labels=["Disabled", "Enabled"],
                value_text_expr="VocabBuilderPlugin::toggleName(value)",
            ),
            PluginsTabEntry(
                label="Vocab TXT File",
                kind="string",
                key="vocabFilePath",
                show_on_device=False,
            ),
        ],
        post_patch=_post_patch,
    )
