#include <iostream>

using namespace std;

// определение шаблона функции min()
// с параметром-типом Type и параметром-константой size
template <typename Type, int size>
   Type min( Type (&r_array)[size] )
   {
      Type min_val = r_array[0];
         for ( int i = 1; i < size; ++i )
               if ( r_array[i] < min_val )
                        min_val = r_array[i];
                           return min_val;
                           }
                           // size не задан -- ok
                           // size = число элементов в списке инициализации
                           int ia[] = { 10, 7, 14, 3, 25 };
                           double da[6] = { 10.2, 7.1, 14.5, 3.2, 25.0, 16.8 };
                           #include <iostream>
                           int main()
                           {
                              // конкретизация min() для массива из 5 элементов типа int
                                 // подставляется Type => int, size => 5
                                    int i = min( ia );
                                       if ( i != 3 )
                                             cout << "??oops: integer min() failed\n";
                                                else cout << "!!ok: integer min() worked\n";
                                                   // конкретизация min() для массива из 6 элементов типа double
                                                      // подставляется Type => double, size => 6
                                                         double d = min( da );
                                                            if ( d != 3.2 )
                                                                  cout << "??oops: double min() failed\n";
                                                                     else cout << "!!ok: double min() worked\n";
                                                                        return 0;
                                                                        }