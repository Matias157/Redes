from socket import *
import threading

class HttpRequest:
	CRLF = '\r\n'																								#define carriage return character e line-feed character
	Socket = socket(AF_INET,SOCK_STREAM)																		#atributo do tipo socket

	def __init__ (self, socket):																					
		self.Socket = socket																					#seta o atributo com o paramtro
		threading.Thread(target=self.run, args=()).start()														#cria uma thread para o metodo run

	def run(self):
		try:																									#run deve capturar quaisquer excecoes que venham a ocorrer em processRequest
			self.processRequest()																				
		except Exception as e:																					
			print(e)																							

	def processRequest(self):																					
		request = self.Socket.recv(1024)																		#coloca a requisicao recebida no socket na variavel request
		print(request)																							#mostra a requisicao que foi recebida

		requestLine = request[:request.find(self.CRLF)]															#request line e tudo que temos antes do primeiro CRLF
		headerLine = request[request.find(self.CRLF)+2:]														#header line e tudo que temos apos o primeiro CRLF
		fileName = requestLine[5:requestLine.find(" HTTP/1.1")]													#o nome do arquivo requisitado esta sempre no request line entre o GET e o HTTP/1.1

		fileExists = True																						#define um bool para verificarmos erros de arquivos nao existentes
		try:																									
			f = open(fileName)																					#tenta abrir o arquivo
			f.close()																							#fechamos o arquivo logo em seguida (considerado uma boa pratica, pois e dificil saber quando o runtime do python ira faze-lo)
		except Exception as e:																					
			fileExists = False																					#caso ocorra a excecoes a veriavel e setada para falso

		if(fileExists):																							#caso o aqruivo exista
			statusLine = "HTTP/1.0 200 OK" + self.CRLF															#criamos o status line com codigo de status OK
			contentTypeLine = "Content-type: " + self.contentType(fileName) + self.CRLF + self.CRLF				#criamos apenas uma linha de header com o content type (e preciso coloca dois CRLF para indicar que essa e ultima linha do header)
			with open(fileName, 'r') as myfile:																	#abrimos o araquivo solicitado
				entityBody = myfile.read()																		#passamos os bits do arquivo para uma string
		else:																									#caso o arquivo nao exista
			statusLine = "HTTP/1.0 404 Not Found" + self.CRLF													#criamos o status line com codigo de status Not Found
			contentTypeLine = "Content-type: " + "text/html" + self.CRLF + self.CRLF							#como a pegina de erro eh sempre a mesma o content type sera sempre text/html
			entityBody = "<HTML>" + "<HEAD><TITLE>Not Found</TITLE></HEAD>" + "<BODY>Not Found</BODY></HTML>"	#string contendo o codigo html da pagina de erro

		print(statusLine + contentTypeLine + entityBody)														#mostramos a resposta que sera enviado
		self.Socket.send(statusLine + contentTypeLine + entityBody)												#enviamos a resposta concatenando o staus line, header e a string do arquivo

		self.Socket.close()																						#fechamos conexao

	def contentType(self, fileName):																			#metodo que cria o content type baseado na extensao do aqruivo
		if(fileName.split(".")[-1] == "html" or fileName.split(".")[-1] == "htm"):								#pegamos apenas a substring apos a virgula e testemos se e html ou hml
			return("text/html")																					#retornamos o content type para html
		elif(fileName.split(".")[-1] == "jpg"):																	#testasmos se e jpg
			return("image/jpg")																					#retornamos o content type para jpg
		elif(fileName.split(".")[-1] == "gif"):																	#testemos se e gif
			return("image/gif")																					#retornamos o content type para gif
		else:																									#caso seja uma extensao desconhecida
			return("application/octet-stream")																	#retornamos o content type para extensoes desconhecidas

serverPort = 1313																								#definimos a porta do servidor
serverSocket = socket(AF_INET,SOCK_STREAM)																		#criamos o socket TCP de entrada
serverSocket.bind(('',serverPort))																				#associamos a porta do servidor ao socket
serverSocket.listen(1)																							#ficamos escutando ate que um cliente bata
print("The server is ready to receive")																			#informamos que o servidor esta ativo
while(True):																									#ficamos esperando possiveis conexos
	connectionSocket, addr = serverSocket.accept()																#escuta as requisicoes TCP do cliente 
	request = HttpRequest(connectionSocket)																		#associa o socket a um objeto de HttpRequest para processar possiveis requisicoes
