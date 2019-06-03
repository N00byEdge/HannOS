namespace HannOS {
  constexpr auto isDebug =
#ifdef NDEBUG
    false;
#else
    true;
#endif
}
