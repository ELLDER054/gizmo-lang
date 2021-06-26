import unittest
import subprocess
import tempfile
import os
import re

class CompiledTest(unittest.TestCase):

    def setUp(self):
        (_, self.ipath) = tempfile.mkstemp()
        (_, self.opath) = tempfile.mkstemp()

    def compile(self, code):
        with open(self.ipath, "w") as f:
            f.write(code)

        p = subprocess.run(['build/gizmo', self.ipath, '-o', self.opath], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        
        return p

    def tearDown(self):
        os.remove(self.ipath)
        os.remove(self.opath)


class CompileTimeFailTest(CompiledTest):

    def compile(self, code, expectedError):
        p = super().compile(code)
        self.assertRegex(p.stderr, expectedError)
        self.assertNotEqual(0, p.returncode)


class CompileTimeSucceedTest(CompiledTest):

    def compile(self, code):
        p = super().compile(code)
        self.assertEqual(0, p.returncode)
        

class RunTimeTest(CompiledTest):

    def execute(self, code, regex):
        p = self.compile(code)
        self.assertEqual(0, p.returncode, p.stderr)

        p = subprocess.run(['lli', self.opath], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        self.assertEqual(0, p.returncode)

        self.assertRegex(p.stdout, regex)

