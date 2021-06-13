import unittest
import subprocess
import tempfile
import os

class CompileTimeTest(unittest.TestCase):

    def setUp(self):
        (_, self.ipath) = tempfile.mkstemp()
        (_, self.opath) = tempfile.mkstemp()

    def compile(self, code, expectedError):
        with open(self.ipath, "w") as f:
            f.write(code)

        p = subprocess.run(['build/gizmo', self.ipath, self.opath], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

        self.assertRegex(p.stderr, expectedError)
        self.assertNotEqual(0, p.returncode)
        
    def tearDown(self):
        os.remove(self.ipath)
        os.remove(self.opath)

class RunTimeTest(unittest.TestCase):
    pass

class TestSimpleBadSyntax(CompileTimeTest):

    def test_missing_semicolon(self):
        self.compile(
            "write(\"hello\")",
            "Expected semi-colon to complete statement")
