/* When run, this program prints on stdout a numeric code corresponding to the C++
   standard used by the compiler during compilation
   */
#include <iostream>

int main()
{
   std::cout << __cplusplus << std::endl;
}
