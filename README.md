# Controladores de dispositivo
Este repositorio alberga el código correspondiente a la primera práctica de la asignatura `Programación Hardware`, correspondiente al desarrollo de un controlador de dispositivo.
## Detección del dispositivo usando las estructuras file e inode
Tal y como está el código, lo único que se hace es obtener el minor usando la macro `iminor(inode*)` definida en `linux/kernel.h` para obtener el minor, se almacena su dirección de memoria en el atributo `private_data` de la estructura de datos `file`, y posteriormente se lee este valor almacenado desde la función `read()`.
## Implementada la transferencia de información entre el espacio de memoria del kernel y el espacio de memoria del usuario
Actualmente, se consigue transferir una cadena de 256 bytes (tamaño fijado por mi) y hay que limpiarla en cada transferencia. Hay que optar por otra forma de hacer la transferencia de información entre el espacio de direcciones de usuario y kernel más eficiente.
[Codigo de ejemplo file e inode](https://www.oreilly.com/library/view/linux-device-drivers/0596005903/ch03.html)
