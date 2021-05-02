# The Gizmo Programming Language
Gizmo is a fast and strongly typed language, and sometimes those types of languages are a bit more intimidating than languages like Python or JavaScript. This is why the main priority of Gizmo is to be approachable and easy to write in.

Take this example:

```cpp
class Bird() {
    init() {
        string this.home = "New York";
    }
    
    none flyTo(string city) {
        write("Flying to " + city);
        this.home = city;
    }
}
Bird bird = new Bird();
write(bird.city);
bird.flyTo("New Jersey");
write(bird.city);
```
