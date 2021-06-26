from . import RunTimeTest

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
