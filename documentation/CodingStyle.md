Coding Style
-------------
None at the moment. But trying to find a decent one that we can use.

However, take account of the following:


## Functions
Functions should either return nothing through `void`, or return a status. This status should be an `int`:

   -1  Error
    0  Success
    
    int createThings(void) {
        if (somethingbad) {
            return -1;
        }
        
        // success
        return 0;
    }

    
### Function arguments
Function arguments should be separated by in-arguments and out-arguments. in-arguments should be made `const` as much 
as possible.

    char *copyString(const char *str) {
        ...
    }


### Internal functions
Any function that is internal to the current C-file, should be defined `static`. Underscoring of such a function is 
optional.

    static void _do_this() { ... }
    
## Pointer functions
Pointer functions are functions that return a pointer. If an error occurs, it should return NULL.

    t_object *getReferencedObject(const t_object *src) {
        if (! src) {
            return NULL;
        }
        
        return src->reference;
    }

## Boolean functions
Boolean functions are functions that should not error, but merely return a true or false value. In these cases, int `0` 
will be returned for `false`, and `1` for `true`.
Most of the time, these functions will start with `is` or `has` in their name.

    int isImmutableMethod(t_method *method) {
        if (method->immutable) return 1;
        
        return 0;
    }
