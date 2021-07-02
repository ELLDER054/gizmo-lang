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

    def test_outside_return(self):
        self.compile(
            "return 5;",
            "Can't have return statement outside of function")

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

    def test_array(self):
        self.compile(
            "int[] a = [1, 2, 3, 4, 5];" +
            "write(a[0]);" +
            "write(a[3]);")
    
    def test_mulit_dimensional_array(self):
        self.compile(
            "int[][] a = [[1, 2], [3, 4], [5, 6]];" +
            "write(a[0][1]);")

    def test_strcat(self):
        self.compile(
            "write(\"abcdef\" + \"ghijkl\" + \"mnopqr\");")

    def test_function_with_no_args(self):
        self.compile(
            "int a() {\n\treturn 5;\n}\nwrite(a());")

    def test_function_with_args(self):
        self.compile(
            "int a(int x, int y) {\n\treturn x + y;\n}\nwrite(a(5, 6));")

    def test_function_with_no_return(self):
        self.compile(
            "int a() {}\nwrite(a());")
