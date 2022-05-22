Erzeugung der Ausfuehrbaren Datei: getestet mit Ubuntu 7.10; 8.04; SuSE 10.3

1. Auspacken der Datei.
   tar -xfv linnwtxxx.tar (CR)
2. In das Verzeichnis der Quellen wechseln.
   cd linnwt4 (CR)
3. qmake -v (CR)
   Ergebnis muss Version QT4.x.x sein. Wenn ja:
4  qmake (CR)
   sonst: QT4 suchen und mit vollem Pfad aufrufen z.B.:
   /usr/local/QT4/bin/qmake (CR)
   ein Makefile wird erzeugt. Mit dem Befehl:
5. make
   wird das ganze kompiliert.
6. Die umbenannte Datei in irgend ein BIN Verzeichnis kopieren.
   Das ist so ueblich unter Linux. Nur so wird das Programm auch gefunden
   beim Aufruf. Wie bei Windows sucht auch Linux die ausfuehrbaren Dateien
   in definierten Pfaden.
7. su - (CR)                 bei SuSE 10.3
   cp linnwt /usr/bin/       bei SuSE 10.3
   oder
   sudo cp linnwt /usr/local/bin/   bei UBUNTU
   oder
   sudo cp linnwt /usr/bin/   bei UBUNTU
8. Soll die ungarische Uebersetzung mit aufgerufen werden muss die 
   "app_hu.qm" angehangen werden. Funktioniert die Uebrsetzung nicht
   so ist es erforderlich den Pfad mit anzugeben.
   z.B.:
   
   /usr/local/bin/linnwt /home/dl4jal/app_hu.qm
   
   --------------------
   fertig