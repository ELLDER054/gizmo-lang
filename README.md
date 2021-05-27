# The Gizmo Programming Language
![gizmo-logo](https://user-images.githubusercontent.com/76635411/117458638-7f045b80-af18-11eb-8bbe-33fb328ea454.png)  
Gizmo is a small, user-friendly programming language.  
See the main website [here](https://ellder054.github.io/gizmolang/index.html) to learn more.  
WARNING: Please be aware that the main website is still in development.

### How Can I Help?
Contributors are always welcome! Some ways to help are:  
1. Fix issues and help design our website.  
2. Create helpful packages and modules for others to use.
3. Get support for gizmo on your favorite IDE if it isn't already supported.  
4. Report any bugs that you find.  
5. Play around with Gizmo and spread the word.
6. Sponsor us on github

## Hacking, Changing or Using Gizmo Locally
Requirements:
1. clang
2. llvm
3. cmake
4. git
5. gcc
6. A terminal or shell

Copy the following commands to build Gizmo.
```shell
git clone https://github.com/ellder054/gizmo-lang.git
mkdir giz-build
cd giz-build
cmake ../gizmo-lang
cmake .
make
```
This makes an executable file called `gizmo`. You can now run gizmo code by doing the following:
```
./gizmo <filename>.gizmo <output>
./<output>
```
