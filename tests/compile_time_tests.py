from . import CompileTimeFailTest, CompileTimeSucceedTest

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

