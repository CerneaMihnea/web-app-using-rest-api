# Cernea Mihnea-Ioan 324CB
client: client.c buffer.c helpers.c parson.c requests.c client_functions.c client_functions.h
	gcc -o client client.c buffer.c helpers.c parson.c requests.c client_functions.c client_functions.h

clean: 
	rm -f *.o client
