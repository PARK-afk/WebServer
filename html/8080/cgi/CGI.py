#!/usr/bin/env python3
import cgi
import cgitb
from list import FileControl
from calculator import Calculator
import sys
import os
import logging
cgitb.enable(display=0, logdir="/logs")

form = cgi.FieldStorage()

def responseForm(status, content, contentType, contentLength, location=None):
	print(f"HTTP/1.1 {status} {status}\r")
	print(f"Content-Type: {contentType}\r")
	if location:
		print(f"Location: {location}\r")
	print(f"Content-Length: {contentLength}\r")
	print("\r")
	print(content)

def HTMLForm(message):
	htmlstring = f"""
<html>
	<head>
	<title>CGI</title>
	</head>
	<body>
		<h1>{message}</h1>
	</body>
</html>
	"""
	return htmlstring


def handleGet():
	path_info = os.environ.get('PATH_INFO')
	if path_info == "/calculator":
		cal = Calculator(form)
		cal.calculate()
		cal.display()
	elif path_info == "/listing":
		listing = FileControl(form)
		listing.display()
	else:
		html = HTMLForm("Bad Request METHOD")
		responseForm(400, html, "text/html", len(html))




def handlePost():
	contetenType = os.environ.get('CONTENT_TYPE')
	if contetenType.startswith("multipart/form-data"):
		listing = FileControl(form)
		listing.handleUpload()
	else:
		html = HTMLForm("Bad Request POST")
		responseForm(400, html, "text/html", len(html))
	


def handleMethod():
	method = os.environ.get('REQUEST_METHOD')
	if method == 'GET':
		handleGet()
	elif method == 'POST':
		handlePost()
	else:
		html = HTMLForm("Method Not Allowed")
		responseForm(405, html, "text/html", len(html))

def checkProtocol():
	protocol = os.environ.get('SERVER_PROTOCOL')
	if protocol == "HTTP/1.1":
		return True
	else:
		return False


def main():
	
	if not checkProtocol():
		html = HTMLForm("Bad Request")
		responseForm(400, html, "text/html", len(html))
		return 
	handleMethod()

if __name__ == '__main__':
	
	main()


