from flask import Flask
from redis import Redis, RedisError
import os
import socket

# Connect to Redis
redis = Redis(host=os.environ.get('REDIS_HOST', 'redis'), port=6379)

app = Flask(__name__)

@app.route("/")
def hello():

  redisCount = 0
  visits = 0

  path = "/home/redis/count.txt"

  if os.path.isfile(path):
    with open(path,'r') as f:
      visits = int(f.readline())
  else:
    with open(path,'w') as f:
      f.write('0\n')
      print('file written')

  try:
    redisCount = redis.incr("counter")
    with open(path,'w') as f:
      visits += 1
      f.write('%d\n' % visits)
      print('file written')

  except RedisError:
    visits = "<i>cannot connect to Redis, counter disabled</i>"
    redisCount = "<i>not connected to Redis</i>"


  html = "<h3>Hello {name}!</h3>" \
         "<b>Hostname:</b> {hostname}<br/>" \
         "<b>Total Visits:</b> {visits}<br/>" \
         "<b>Visits since connected to Redis:</b> {count}"

  return html.format(name=os.getenv("NAME", "world"), hostname=socket.gethostname(), visits=visits, count=redisCount)

if __name__ == "__main__":
  app.run(host='0.0.0.0', port=80)
