from . import RunTimeTest

class SimpleRunTimeTests(RunTimeTest):
    
    def test_simple_write_string(self):
        self.execute(
            "write(\"hello\");",
            "hello")

    def test_simple_write_int(self):
        self.execute(
            "write(5);",
            "5")
            
    def test_simple_write_char(self):
        self.execute(
            "write('a');",
            "a")

    def test_simple_write_real(self):
        self.execute(
            "write(5.5);",
            "5.5")

    def test_simple_write_bool(self):
        self.execute(
            "write(true);",
            "true")

    def test_strcat(self):
        self.execute(
            "write(\"abc\" + \"def\");",
            "abcdef")

    def test_expr(self):
        self.execute(
            "write(3 * 6 / 2);",
            "9")

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
