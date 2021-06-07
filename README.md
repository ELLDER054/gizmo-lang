[![GitHub license](https://img.shields.io/github/license/ELLDER054/gizmo-lang.svg)](https://github.com/ELLDER054/gizmo-lang/blob/master/LICENSE)
[![Maintenance](https://img.shields.io/badge/Maintained%3F-yes-green.svg)](https://GitHub.com/ELLDER054/gizmo-lang/graphs/commit-activity)

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

## Getting a Weird Error?
Some of the "weird" errors that you might get include:  
1.
  ```
gizmo: This feature (number) is either not yet implemented in the back-end
or there is an internal compiler error.
Please report this error, along with the number in the parenthesis, to the developers at gizmo@gizmolang.org
  ```
2.
  ```
gizmo: Compiler failed to generate llvm ir code
  ```

The first error means that the feature you are trying to use is either only  
implemented halfway, or one of the developers forgot to add a certain line of code somewhere.  
  
The second error means that the code generator failed, and the llvm was generated incorrectly.  
