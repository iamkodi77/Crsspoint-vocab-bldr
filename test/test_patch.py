#!/usr/bin/env python3
"""
Unit and regression tests for Vocabulary Builder patcher and installer.
"""

import os
import shutil
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from patch import patch_crosspoint_reader
from install import install_direct_to_crosspoint, install_to_xteink_plugins


class TestVocabBuilderPatch(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.crosspoint_template = "/tmp/crosspoint-reader"
        cls.xteink_template = "/tmp/xteink-plugins"

    def setUp(self):
        self.scratch_dir = tempfile.mkdtemp(prefix="test_vocab_patch_")

    def tearDown(self):
        if os.path.exists(self.scratch_dir):
            shutil.rmtree(self.scratch_dir)

    def test_patch_crosspoint_reader(self):
        if not os.path.isdir(self.crosspoint_template):
            self.skipTest("crosspoint-reader template not found in /tmp")

        repo_copy = os.path.join(self.scratch_dir, "crosspoint-reader")
        shutil.copytree(self.crosspoint_template, repo_copy)

        # Apply patch
        patch_crosspoint_reader(repo_copy)

        # Verify ActivityResult.h
        with open(os.path.join(repo_copy, "src", "activities", "ActivityResult.h")) as f:
            c = f.read()
            self.assertIn("Action : uint8_t { Clip, Lookup, Bookmark, Vocab }", c)

        # Verify ClipSelectionActivity.h
        with open(os.path.join(repo_copy, "src", "activities", "reader", "ClipSelectionActivity.h")) as f:
            c = f.read()
            self.assertIn("4 - firstAction()", c)

        # Verify ClipSelectionActivity.cpp
        with open(os.path.join(repo_copy, "src", "activities", "reader", "ClipSelectionActivity.cpp")) as f:
            c = f.read()
            self.assertIn("ClippingResult::Action::Vocab", c)
            self.assertIn('renderer.getTextWidth(font, "Vocab")', c)
            self.assertIn("SETTINGS.vocabDefaultAction", c)

        # Verify BaseTheme.cpp
        with open(os.path.join(repo_copy, "src", "components", "themes", "BaseTheme.cpp")) as f:
            c = f.read()
            self.assertIn('"Vocab"', c)

        # Verify EpubReaderActivity.h
        with open(os.path.join(repo_copy, "src", "activities", "reader", "EpubReaderActivity.h")) as f:
            c = f.read()
            self.assertIn("customClippingMessage", c)

        # Verify EpubReaderActivity.cpp
        with open(os.path.join(repo_copy, "src", "activities", "reader", "EpubReaderActivity.cpp")) as f:
            c = f.read()
            self.assertIn("vocabulary/VocabBuilderStore.h", c)
            self.assertIn("clipping.action == ClippingResult::Action::Vocab", c)
            self.assertIn("VocabBuilderStore::saveWord", c)
            self.assertIn("customClippingMessage.clear()", c)
            self.assertIn("!customClippingMessage.empty()", c)

        # Test idempotency - running again should not fail
        patch_crosspoint_reader(repo_copy)

    def test_install_to_xteink_plugins(self):
        if not os.path.isdir(self.xteink_template):
            self.skipTest("xteink-plugins template not found in /tmp")

        xteink_copy = os.path.join(self.scratch_dir, "xteink-plugins")
        shutil.copytree(self.xteink_template, xteink_copy)

        ok = install_to_xteink_plugins(xteink_copy)
        self.assertTrue(ok)

        dest = os.path.join(xteink_copy, "plugins", "vocabbuilder")
        self.assertTrue(os.path.exists(os.path.join(dest, "plugin.py")))
        self.assertTrue(os.path.exists(os.path.join(dest, "VocabBuilderActivity.h")))
        self.assertTrue(os.path.exists(os.path.join(dest, "VocabBuilderStore.cpp")))


if __name__ == "__main__":
    unittest.main()
