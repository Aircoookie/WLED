class BlynkStackOnly
{
protected:
  BlynkStackOnly() {}
  ~BlynkStackOnly() {}

private:
  /// @brief Declared as private to prevent usage of dynamic memory
  void* operator new(size_t size);
  /// @brief Declared as private to prevent usage of dynamic memory
  void operator delete(void *p);
};

class BlynkNonCopyable
{
protected:
  BlynkNonCopyable(){}
  ~BlynkNonCopyable(){}

private:
  /// @brief Declared as private to prevent usage of copy constructor
  BlynkNonCopyable(const BlynkNonCopyable&);
  /// @brief Declared as private to prevent usage of assignment operator
  BlynkNonCopyable& operator=(const BlynkNonCopyable&);
};

template<typename T>
class BlynkSingleton
  : public BlynkNonCopyable
{
public:
  /** @brief Returns the instance of the singleton type
   When called for the first time, the singleton instance will be
   created. All subsequent calls will return a reference to the
   previously created instance.
   @return The singleton instance
   */
  static T* instance()
  {
    static T instance;
    return &instance;
  }
protected:
  BlynkSingleton() {}
  ~BlynkSingleton() {}
};

