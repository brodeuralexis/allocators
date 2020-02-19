#ifndef __ALLOCATORS__MACROS__
#define __ALLOCATORS__MACROS__

/**
 * Tags the given value as unused, preventing the compiler for emitting an error for an unused variable.
 * @param _variable A variable
 */
#define UNUSED(_variable) \
    ((void) (_variable))

/**
 * Given a child type and a field of the child type, using a pointer to the field, calculates the beginning in memory of
 * the child type from a pointer to the parent type.
 * @param _Child A child struct
 * @param _Field A field of the child type
 * @param _parent A pointer to the field
 * @return A pointer to the child type
 */
#define FIELD_PARENT_PTR(_Child, _Field, _parent) \
    ((_Child*) (((char*) _parent) - offsetof(_Child, _Field)))

#endif
