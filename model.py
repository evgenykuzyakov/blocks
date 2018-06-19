import tensorflow as tf

def model(input_data, training=False):
    with tf.variable_scope("model", reuse=tf.AUTO_REUSE):
        # input is padded with walls of width 2
        layer = input_data
        print(layer)
        paddings = tf.constant([[0, 0], [2, 2], [2, 2]])
        layer = tf.pad(layer, paddings, constant_values=1)
        print(layer)
        layer = tf.reshape(tf.cast(layer, tf.float32), [-1, 12, 12, 1])
        print(layer)

        convs = [
            {
                'filters': 64,
                'kernel_size': [5, 5],
                'padding': 'valid',
                'activation': tf.nn.relu,
                'max_pool': None,
                'pool_strides': None,
                'expected_size': 8,
            },
            {
                'filters': 256,
                'kernel_size': [5, 5],
                'padding': 'valid',
                'activation': tf.nn.relu,
                'max_pool': None,
                'pool_strides': None,
                'expected_size': 4,
            },
            {
                'filters': 1024,
                'kernel_size': [4, 4],
                'padding': 'valid',
                'activation': tf.nn.relu,
                'max_pool': None,
                'pool_strides': None,
                'expected_size': 1,
            },
        ]

        dense_units = 1024

        for conv in convs:
            layer = tf.layers.conv2d(
                inputs=layer,
                filters=conv['filters'],
                kernel_size=conv['kernel_size'],
                padding=conv['padding'],
                activation=conv['activation'],
            )
            print(layer)
            if conv['max_pool'] is not None:
                layer = tf.layers.max_pooling2d(
                    inputs=layer,
                    pool_size=conv['max_pool'],
                    pool_strides=conv['pool_strides'],
                )
                print(layer)

        layer = tf.reshape(layer, [-1, convs[-1]['filters']])
        print(layer)

        layer = tf.layers.dense(
            inputs=layer,
            units=dense_units,
            activation=tf.nn.relu,
        )
        print(layer)
        layer = tf.layers.dense(
            inputs=layer,
            units=2,
            activation=None,
        )
        print(layer)
        return {
            'logits': layer,
            'probabilities': tf.nn.softmax(layer)[:, 1],
        }
