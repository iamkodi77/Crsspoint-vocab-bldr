#!/usr/bin/env python3
"""
Integration test for Vocabulary Builder with the xteink-plugins framework.
"""

import os
import shutil
import subprocess
import sys
import tempfile
import unittest


class TestXteinkIntegration(unittest.TestCase):
    def test_vocabbuilder_alone_and_combined(self):
        xteink_dir = "/tmp/xteink-plugins"
        crosspoint_dir = "/tmp/crosspoint-reader"

        if not os.path.isdir(xteink_dir) or not os.path.isdir(crosspoint_dir):
            self.skipTest("xteink-plugins or crosspoint-reader not found in /tmp")

        repo_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        plugin_dest = os.path.join(xteink_dir, "plugins", "vocabbuilder")

        # Copy this repo's files to xteink-plugins
        if os.path.exists(plugin_dest):
            shutil.rmtree(plugin_dest)
        shutil.copytree(
            repo_root,
            plugin_dest,
            ignore=shutil.ignore_patterns(".git", "test", "__pycache__", "*.o"),
        )

        scratch_root = tempfile.mkdtemp(prefix="test_xteink_scratch_")
        try:
            cmd = [
                sys.executable,
                os.path.join(xteink_dir, "test_harness.py"),
                crosspoint_dir,
                scratch_root,
                "vocabbuilder,lockscreen,pong",
            ]
            res = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            self.assertEqual(res.returncode, 0, f"test_harness failed: {res.stdout}\n{res.stderr}")
            self.assertIn("OK  vocabbuilder+lockscreen+pong", res.stdout)
        finally:
            if os.path.exists(scratch_root):
                shutil.rmtree(scratch_root)


if __name__ == "__main__":
    unittest.main()
