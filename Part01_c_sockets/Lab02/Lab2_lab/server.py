import pyodbc
import socket
from threading import Thread


class ClientThread(Thread):
    def __init__(self, conn, ip, port):
        Thread.__init__(self)
        self.conn = conn
        self.ip = ip
        self.port = port
        print("S a creat worker-ul pentru "+str(self.ip))

    def run(self):
        patients = self.getPatients()
        self.conn.send(bytes(patients, 'utf8'))


    def getPatients(self):
        conn = pyodbc.connect('Driver={SQL Server};'
                          'Server=DESKTOP-ALKE6K0;'
                          'Database=Hospital;'
                          'Trusted_Connection=yes;')

        cursor = conn.cursor()
        cursor.execute('SELECT * FROM Patient')

        patients=""
        for row in cursor:
            patients+=row[1]+" "
        return patients



def main():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind(("192.168.1.6", 1234))

    server.listen(5)
    
    threadList = []
    while True:
        (conn, (ip, port)) = server.accept()
        clientWorker = ClientThread(conn, ip, port)
        clientWorker.start()
        threadList.append(clientWorker)

    for thread in threadList:
        thread.wait()

main()
