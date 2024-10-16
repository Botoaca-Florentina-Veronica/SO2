# SO1

Informații adiționale:
1. Pentru a genera un fișier cu conținut aleator se poate folosi comanda dd:
         dd if=/dev/urandom of=<nume-fișier-ieșire> bs=1000 count=<număr-dorit>
2. Pentru citirea și scrierea fișierelor se va folosi open + read/write + close.
3. Pentru accesarea informațiilor din inode se va folosi stat/lstat/fstat.
4. Pentru testarea programului se creează un arbore de directoare, fișiere și legături simbolice folosind mkdir, touch și ln -s.
5. Pentru a construi informații în mod predefinit, se recomandă snprintf.
