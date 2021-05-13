# The Gizmo Programming Language
![gizmo-logo](https://user-images.githubusercontent.com/76635411/117458638-7f045b80-af18-11eb-8bbe-33fb328ea454.png)  
Gizmo is a small, user-friendly programming language.  
See the main website [here](https://ellder054.github.io/gizmolang/) to learn more.  
WARNING: Please be aware that the main website is still in development.

### How Can I Help?
Contributors are always welcome! Some ways to help are:  
1. Fix issues and help design our website.  
2. Create helpful packages and modules for others to use. See [package design](https://ellder054.github.io/gizmolang/packages.html).  
3. Get support for gizmo on your favorite IDE if it isn't already supported.  
4. Report any bugs that you might find.  
5. Play around with Gizmo and spread the word.
6. Sponsor me on github

## Hacking, Changing or Using Gizmo Locally
Warning: If you build Gizmo now, there is a chance that Gizmo will be updated and you will not have the most recent version.  
1. Make sure git is installed in your terminal.  
2. Make sure cmake is installed in your terminal.
3. Run this code in your terminal to build Gizmo.
```shell
git clone https://github.com/ellder054/gizmo-lang.git
mkdir gizmo-build
cd gizmo-build
cmake ../gizmo-lang
make
```

### Compiling a file
Make sure you have built Gizmo and go to the "gizmo-build" directory.  
Example files can be found in the `examples` folder.  
You can now tell Gizmo to compile a Gizmo file with the following.
```shell
./gizmo filename.gizmo output
```
Replace `filename.gizmo` with whatever file you are trying to compile and replace `output` with your output file.  
Once you have compiled, you should be able to run this code to execute your file:
```shell
./output
```
Again, replace `output` with your output file.
