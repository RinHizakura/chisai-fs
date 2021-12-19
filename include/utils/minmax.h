/* Reference:
 * https://github.com/torvalds/linux/blob/master/include/linux/minmax.h */
#ifndef MINMAX_H
#define MINMAX_H

/* extra dependent macro */
#define ___PASTE(a, b) a##b
#define __PASTE(a, b) ___PASTE(a, b)
#define __UNIQUE_ID(prefix) __PASTE(__PASTE(__UNIQUE_ID_, prefix), __COUNTER__)
/*
 * Here's some C standard you should know to understand this macro:
 * 1. An integer constant expression with the value 0, or such an expression
 * cast to type void *, is called a null pointer constant.
 * 2. For conditional operator(?:), if both operands are pointers to compatible
 * types or to differently qualified versions of compatible types, the result
 * type is a pointer to an appropriately qualified version of the composite
 * type.
 * 3. For conditional operator(?:), if one operand is a null pointer constant,
 * the result has the type of the other operand.
 */
#define __is_constexpr(x) \
    (sizeof(int) == sizeof(*(8 ? ((void *) ((long) (x) *0l)) : (int *) 8)))
/*
 * min()/max()/clamp() macros must accomplish three things:
 *
 * - avoid multiple evaluations of the arguments (so side-effects like
 *   "x++" happen only once) when non-constant.
 * - perform strict type-checking (to generate warnings instead of
 *   nasty runtime surprises). See the "unnecessary" pointer comparison
 *   in __typecheck().
 * - retain result as a constant expressions when called with only
 *   constant expressions (to avoid tripping VLA warnings in stack
 *   allocation usage).
 */
#define __typecheck(x, y) (!!(sizeof((typeof(x) *) 1 == (typeof(y) *) 1)))

#define __no_side_effects(x, y) (__is_constexpr(x) && __is_constexpr(y))

#define __safe_cmp(x, y) (__typecheck(x, y) && __no_side_effects(x, y))

#define __cmp(x, y, op) ((x) op(y) ? (x) : (y))

#define __cmp_once(x, y, unique_x, unique_y, op) \
    ({                                           \
        typeof(x) unique_x = (x);                \
        typeof(y) unique_y = (y);                \
        __cmp(unique_x, unique_y, op);           \
    })

#define __careful_cmp(x, y, op)            \
    __builtin_choose_expr(                 \
        __safe_cmp(x, y), __cmp(x, y, op), \
        __cmp_once(x, y, __UNIQUE_ID(__x), __UNIQUE_ID(__y), op))

/**
 * min - return minimum of two values of the same or compatible types
 * @x: first value
 * @y: second value
 */
#define min(x, y) __careful_cmp(x, y, <)

/**
 * max - return maximum of two values of the same or compatible types
 * @x: first value
 * @y: second value
 */
#define max(x, y) __careful_cmp(x, y, >)

#endif
