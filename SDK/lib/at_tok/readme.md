### EXPERIMENTAL FEATURE

Add & Compile ( makefile style ) not compiled sources with main builder

```ini
board_build.modules = 
    $FRAMEWORK_DIR/SDK/lib/at_tok ; is folder - search for "build.py" 
    $PROJECT_DIR/modules/module-drive-rover-on-mars/builder-script.py
```

```python
from os.path import join

def module_init(env):
    print("  AT TOKENIZER")
    env.Append(
        CPPPATH = [ join( env.framework_dir, "SDK", "lib", "at_tok" ), ]           
    )     
    env.BuildSources( join( "$BUILD_DIR", "modules", "at_tok" ), join( env.framework_dir, "SDK", "lib", "at_tok" )  )
    
    # or BuildLibrary()
```
