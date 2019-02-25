# Controladores de dispositivo
Este repositorio alberga el código correspondiente a la primera práctica de la asignatura `Programación Hardware`, correspondiente al desarrollo de un controlador de dispositivo.
## Detección del dispositivo usando las estructuras file e inode
Tal y como está el código, lo único que se hace es obtener el minor usando la macro `iminor(inode*)` definida en `linux/kernel.h` para obtener el minor, se almacena su dirección de memoria en el atributo `private_data` de la estructura de datos `file`, y posteriormente se lee este valor almacenado desde la función `read()`.
## Implementada la transferencia de información entre el espacio de memoria del kernel y el espacio de memoria del usuario
Actualmente, se consigue transferir una cadena de 256 bytes (tamaño fijado por mi) y hay que limpiarla en cada transferencia. Hay que optar por otra forma de hacer la transferencia de información entre el espacio de direcciones de usuario y kernel más eficiente. SOLUCIONADO: añadir el caracter 0 al final de la cadena leída.
[Codigo de ejemplo file e inode](https://www.oreilly.com/library/view/linux-device-drivers/0596005903/ch03.html)
## Diseño final.
Finalmente van a implementarse tres drivers: 
Clase: DriverMiguelClass

- [x] **led**: servirá para encender o apagar los LEDs del teclado mediante argumentos. <br/>
- [ ] **fibonacci**: podremos obtener una sucesión de fibonacci de tantos números como se especifiquen mediante parámetro (el número de bytes a leer indicará cuantos números queremos.)
- [x] **save**: espacio de almacenamiento de 64K simulando un disco duro de los años 80 (en cuanto a tamaño).

