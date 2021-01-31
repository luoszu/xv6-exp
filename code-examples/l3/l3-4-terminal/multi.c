#include "types.h"
#include "user.h"
#define SIZE 400
int main(int argc, char** argv)
{
	char str[] = "";
	for (int k = 1; k <= SIZE; ++k) {
		for (int i = 1; i <= SIZE; ++i) {
			for (int j = 1; j <= SIZE; ++j) {
				strcpy(str, "multi_user");
			}
		}	
	}
  	printf(1, "rec is hello\n");
	exit();
}
