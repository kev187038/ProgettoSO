# ProgettoSO

In h_files/mmu.h ho definito i campi che utilizzerò: in particolare ho pensato a creare delle struct apposite per simulare entry della tabella delle pagine, frame fisici, memoria fisica RAM e disco.

Assumo per ora di dover allocare una sola tabella delle pagine all'inizio della memoria fisica, stando a ciò che è scritto nella consegna.
L'algoritmo second chance implementato è il seguente:
  -scorro ogni elemento della lista circolare fino a trovare una entry valida con bit R e W a 0 e sostituisco quella(candidato ideale)
  -per ogni elemento valido incontrato con R o W a 1, li setto a 0(second chance), e passo avanti
  -per ogni elemento valido incontrato con R e W a 1, setto W a 0 e passo avanti
  -per ogni elemento valido incontrato che non sia il candidato ideale, lo sposto alla fine della lista 
  
  L'algoritmo presume che tutta la memoria sia occupata fin dall'inizio.
  

Hello there! What this little project is all about, is simulating in a software how a MMU works. Both physical as well as logical aspects of the virtual memory management system are represented. The MMU operates with 16MB of logical memory that need to share 1MB of physical memory. The swapping process is done through a "swapping file" of 16MB of space and the algorithm used for page fault handling is the enhanced second chance algorithm.
