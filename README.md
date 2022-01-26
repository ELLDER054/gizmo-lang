# FYI
The Gizmo Programming Language Project has moved from this repo to [this](https://github.com/ELLDER054/rusty-gizmo) repo

**Gizmo** is a fast and user friendly programming language for parsing and manipulating strings.  
Gizmo code looks like this:  
```cpp
int add(int a, int b) {
  return a + b;
}
int x = 0;
write(add(x, 6));
```

See the [main website](http://www.gizmolang.org) to learn more!

## Building Gizmo

Requirements:
1. A terminal or shell
2. gcc
3. make and cmake
4. llvm

Execute the following commands in your terminal or shell.

```shell
git clone https://github.com/ELLDER054/gizmo-lang.git
cd gizmo-lang
mkdir build
cd build
cmake ..
make
```

You can then put the `gizmo` file into `/usr/local/bin/` to be able to use the `gizmo` command.

## Contributing
Contributing is welcome! Some of the best ways to help are:

1. Submit a pull request and add a cool new feature
2. Open an issue or let us know about bugs
3. Fix parts of the README and website
4. Make packages for other Gizmo programmers to use
5. Spread the word
6. Sponsor us here on github

## I Want To Start Learning!
That's the enthusiasm we want to hear!  
First, see the [installation guide](http://www.gizmolang.org/install.html).  
Then, go see the [documentation](http//www.gizmolang.org/docs.html) or the [wiki](https://github.com/ELLDER054/gizmo-lang/wiki)!
