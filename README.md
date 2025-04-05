# SO

Pretende-se implementar um serviço que permita a indexação e pesquisa sobre documentos de texto guardados localmente num computador. O programa servidor é responsável por registar meta-informação sobre cada documento (p.ex, identificador único,
título, ano, autor, localização), permitindo também um conjunto de interrogações relativamente a esta meta-informação e ao conteúdo dos documentos.


Os utilizadores devem utilizar um programa cliente para interagir com o serviço (i.e., com o programa servidor). Esta interação permitirá que os utilizadores adicionem ou removam a indexação de um documento no serviço, e que efetuem pesquisas (interrogações) sobre os documentos indexados. De notar que o programa cliente apenas executa uma operação por invocação, ou seja, não é um programa interativo (i.e., que vai lendo várias operações a partir do stdin)
