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

class TestSimpleBadSyntax(CompileTimeFailTest):

    def test_missing_semicolon(self):
        self.compile(
            "write(\"hello\")",
            "Expected semi-colon to complete statement")

    def test_redefinition(self):
        self.compile(
            "int a = 5;" + 
            "int a = 5;",
            "Redefinition of variable `a`")

class TestSimpleGoodSyntax(CompileTimeSucceedTest):

    def test_simple_write(self):
        self.compile(
            "write(\"hello\");")

    def test_single_var_decl(self):
        self.compile(
            "int a = 5;")

    def test_double_var_decl(self):
        self.compile(
            "int a = 5;" +
            "int b = 6;")

class SimpleRunTimeTests(RunTimeTest):
    
    def test_simple_output(self):
        self.execute(
            "write(\"hello\");",
            "hello")

    def test_else_if(self):
        expectedOutput = "i is 0\n" + \
                         "i is 1\n" + \
                         "i is 2\n" + \
                         "i is 3\n"

        self.execute(
            """
            int i = 0;

            while (i < 4) {
                if (i == 0) {
                    write("i is 0\\n");
                } else if (i == 1) {
                    write("i is 1\\n");
                } else if (i == 2) {
                    write("i is 2\\n");
                } else {
                    write("i is 3\\n");
                }
                i = i + 1;
            }""",
            expectedOutput)
