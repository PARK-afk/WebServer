#!/usr/bin/env python3

import os
import cgi
import datetime
import uuid


UPLOAD_DIR = "/Users/seodong-gyun/42/WebServer/html/8080/at"
MAX_FILE_SIZE = 1024 * 1024 * 10
ALLOWED_EXTENSIONS = [".jpg", ".jpeg", ".png", ".gif", ".txt", ".pdf"]
FILE_SIZE_ERROR = 1
FILE_EXTENSION_ERROR = 2
FILE_UPLOAD_ERROR = 3
FILE_NMAE_ERROR = 4


def responseForm(status, content, contentType, contentLength, location=None):
	print(f"Content-Type: {contentType}")
	if location:
		print(f"Location: {location}")
	print(f"Content-Length: {contentLength}")
	print(f"Status: {status}")
	print()
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

class FileControl:
    
	def __init__(self, form: cgi.FieldStorage):

		self.form = form
		self.fileList = self.listFiles()

	def listFiles(self):
		files = os.listdir(UPLOAD_DIR)
		fileList = ""
		for file in files:
			fileList += f"<li><a href='/uploads/{file}' target='_blank'>{file}</a> <form action='/CGI.py' method='get' style='display: inline;'><input type='hidden' name='delete' value='{file}'><input type='submit' value='삭제'></form></li>"
		return fileList


	def display(self):
		print("HTTP/1.1 200 OK")
		print("Content-type: text/html; charset=utf-8")
		print(f"Date: {datetime.datetime.now().strftime('%a, %d %b %Y %H:%M:%S %Z')}")
		Response = f"""
		<!DOCTYPE html>
		<html lang="ko">
		<head>
			<meta charset="UTF-8">
			<meta name="viewport" content="width=device-width, initial-scale=1.0">
			<title>파일 업로드 및 삭제</title>
			<style>
				body {{
					font-family: Arial, sans-serif;
					background-image: url("ground2.png");
					background-color: #f2f2f2;
					display: flex;
					justify-content: center;
					align-items: center;
					height: 100vh;
					margin: 0;
				}}
				.container {{
					background-color: white;
					padding: 40px;
					border-radius: 10px;
					box-shadow: 0 0 20px rgba(0, 0, 0, 0.1);
					text-align: center;
					max-width: 600px;
				}}
				h1 {{
					color: #333;
					font-size: 36px;
					margin-bottom: 20px;
				}}
				form {{
					margin-top: 20px;
				}}
				input[type="file"] {{
					margin-bottom: 10px;
				}}
				input[type="submit"] {{
					padding: 12px 24px;
					background-color: #333;
					color: white;
					border: none;
					border-radius: 5px;
					cursor: pointer;
					transition: background-color 0.3s ease;
				}}
				input[type="submit"]:hover {{
					background-color: #666;
				}}
				ul {{
					list-style-type: none;
					padding: 0;
					margin-top: 20px;
				}}
				li {{
					margin-bottom: 10px;
				}}
				a {{
					color: #333;
					text-decoration: none;
				}}
				a:hover {{
					text-decoration: underline;
				}}
				.btn {{
					display: inline-block;
					padding: 12px 24px;
					background-color: #333;
					color: white;
					text-decoration: none;
					border-radius: 5px;
					transition: background-color 0.3s ease;
					margin-top: 20px;
				}}
				.btn:hover {{
					background-color: #666;
				}}
			</style>
		</head>
		<body>
			<div class="container">
				<h1>파일 업로드 및 삭제</h1>
				<form action="/CGI.py" method="post" enctype="multipart/form-data">
					<input type="file" name="file">
					<input type="submit" value="업로드">
				</form>
				<h2>업로드된 파일 목록</h2>
				<ul>
					{self.fileList}
				</ul>
				<a href="/features" class="btn">뒤로 가기</a>
			</div>
		</body>
		</html>"""
		print(f"Content-Length: {len(Response)}\r")
		print("Connection: keep-alive\r")
		print("\r")
		print(Response)

	def save_uploaded_file(self, fileitem, UPLOAD_DIR):
		filename = fileitem.filename

		if not filename:
			return FILE_NMAE_ERROR

		_, ext = os.path.splitext(filename)
		if ext.lower() not in ALLOWED_EXTENSIONS:
			return None

		filesize = fileitem.limit
		if filesize > MAX_FILE_SIZE:
			return FILE_SIZE_ERROR

		basename = os.path.basename(filename)

		unique_filename = str(uuid.uuid4()) + "_" + basename

		filepath = os.path.join(UPLOAD_DIR, unique_filename)

		# 파일 저장
		with open(filepath, "wb") as f:
			f.write(fileitem.file.read())

		return unique_filename

	def handleUpload(self):

		fileitem = self.form["file"]

		filename = self.save_uploaded_file(fileitem, UPLOAD_DIR)
		if filename:
			self.fileList = self.listFiles()
			self.display()
		else:
			if filename == FILE_SIZE_ERROR:
				html = """
				<!DOCTYPE html>
				<html>
				<head>
					<title>파일 크기 초과</title>
					<style>
						body {
							font-family: Arial, sans-serif;
							text-align: center;
							padding: 50px;
						}
						h1 {
							font-size: 36px;
							color: #ff6b6b;
							margin-bottom: 20px;
						}
						p {
							font-size: 18px;
							color: #4ecdc4;
						}
					</style>
				</head>
				<body>
					<h1>와우! 너무 크고 엄청나...!!!</h1>
					<p> 파일이 너무 커서 업로드 할 수 없어엇...!</p>
				</body>
				</html>
				"""
				responseForm(400, html, "text/html", len(html))
			elif filename == FILE_EXTENSION_ERROR:
				html = """
				<!DOCTYPE html>
				<html>
				<head>
					<title>허용되지 않는 확장자</title>
					<style>
						body {
							font-family: Arial, sans-serif;
							text-align: center;
							padding: 50px;
						}
						h1 {
							font-size: 36px;
							color: #ff6b6b;
							margin-bottom: 20px;
						}
						h2 {
							font-size: 24px;
							color: #ff6b6b;
							margin-bottom: 20px;
						}
						p {
							font-size: 18px;
							color: #4ecdc4;
						}
						form {
							margin-top: 30px;
						}
						button {
							font-size: 18px;
							padding: 10px 20px;
							background-color: #ff6b6b;
							color: white;
							border: none;
							border-radius: 5px;
							cursor: pointer;
						}
					</style>
				</head>
				<body>
					<h1>앗! 이런 확장자는 허용되지 않아요!</h1>
					<h2>나에게 이런걸 넣는거야...??!!</h2>
					<p>
						허용되는 확장자는 다음과 같습니다: .jpg, .jpeg, .png, .gif, .pdf, .txt, .zip, .tar.gz, .tar.bz2, .tar.xz, .tar</p>
					</p>
					<form>
						<button type="button" onclick="history.back()">뒤로 가기</button>
					</form>
				</body>
				</html>
				"""
				responseForm(400, html, "text/html", len(html))
			else:
				html = HTMLForm("파일 업로드 실패")
				responseForm(400, html, "text/html", len(html))



