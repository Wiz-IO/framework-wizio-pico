#ifndef MINMAX_H_
#define MINMAX_H_
#ifdef __cplusplus

template <class T, class L>
auto min(const T &a, const L &b) -> decltype((b < a) ? b : a)
{
    return (b < a) ? b : a;
}

template <class T, class L>
auto max(const T &a, const L &b) -> decltype((b < a) ? b : a)
{
    return (a < b) ? b : a;
}

extern "C"
{
#endif

#ifndef min
#define min(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })
#endif

#ifndef max
#define max(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#endif

#ifdef __cplusplus
}
#endif
#endif /* MINMAX_H_ */
