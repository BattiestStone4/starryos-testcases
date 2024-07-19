#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#define DATA1 "In Xanadu, did Kublai Khan..."
#define DATA2 "A stately pleasure dome decree..."

main()
{
   int sockets[2], child;
   char buf[1024];
   if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0) {
      perror("opening stream socket pair");
      exit(1);
   }
   if ((child = fork()) == -1)
      perror("fork");
   else if (child) {     /* This is the parent. */
      close(sockets[0]);
      printf("child tid: %d\n", child);
      if (read(sockets[1], buf, 1024, 0) < 0)
         perror("parent: reading stream message");
      printf("parent read: %s\n", buf);
      if (write(sockets[1], DATA2, sizeof(DATA2)) < 0)
         perror("parent: writing stream message");
      else {
        printf("parent wrote \"%s\"\n", DATA2);
      }
      close(sockets[1]);

   } else {     /* This is the child. */
      close(sockets[1]);
      if (write(sockets[0], DATA1, sizeof(DATA1)) < 0)
         perror("child: writing stream message");
    else {
        printf("child wrote \"%s\"\n", DATA1);
    }
      if (read(sockets[0], buf, 1024, 0) < 0)
         perror("child: reading stream message");
      printf("child read: %s\n", buf);
      close(sockets[0]);
   }
}