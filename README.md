# The Gizmo Programming Language
Gizmo is a small, user-friendly programming language.  
See the main website [here](https://ellder054.github.io/gizmolang/) to learn more.

### How Can I Help?
Contributors are always welcome! Some ways to help are:  
1. Fix issues and help design our website.  
2. Create helpful packages and modules for others to use.  
3. Get support for gizmo on your favorite IDE if it isn't already supported.  
4. Report any bugs that you might find.  
5. Play around with Gizmo and spread the word.

## Building Gizmo From Source
Warning: If you build Gizmo now, there is a chance that Gizmo will be updated and you will not have the most recent version.  
Make sure git is installed in your terminal.  
Once you have it installed, run this code in your terminal to build Gizmo.

```shell
git clone https://github.com/ellder054/gizmo-lang
```

The Gizmo source will be put in the directory `gizmo-lang` and you can run Gizmo source code with the following.
```shell
cd gizmo-lang
./main.sh
```
By default code is read from the `test.gizmo` file.  
If you want to change this, enter the `src` directory and enter `compiler.c`.  
Replace all `test.gizmo`'s with what ever file you are trying to execute it on.
