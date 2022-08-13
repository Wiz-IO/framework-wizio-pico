from os.path import join

def module_init(env):
    print("  AT TOKENIZER")
    env.Append(
        CPPPATH = [ join( env.framework_dir, env.sdk, "lib", "at_tok" ), ]           
    )     
    env.BuildSources( join( "$BUILD_DIR", "modules", "at_tok" ), join( env.framework_dir, env.sdk, "lib", "at_tok" )  )