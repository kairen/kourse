# coding=utf-8
import tensorflow as tf

# 目標 Session
server_target = "grpc://140.130.3.6:31235" ##worker service
logs_path = '/tmp/train'

# 指定 worker task 0 使用 CPU 運算
with tf.device("/job:worker/task:0"):
    with tf.device("/cpu:0"):
        a = tf.constant([1.5, 6.0], name='a')
        b = tf.Variable([1.5, 3.2], name='b')
        c = (a * b) + (a / b)
        d = c * a
        y = tf.assign(b, d)

# 運算 Graph
with tf.Session(server_target) as sess:
    sess.run(tf.global_variables_initializer())
    writer = tf.summary.FileWriter(logs_path, graph=tf.get_default_graph())
    print(sess.run(y))

