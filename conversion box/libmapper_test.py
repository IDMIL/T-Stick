import mapper

dev = mapper.device("test_sender")
sensor1 = dev.add_output_signal("sensor1", 1, 'f', "V", 0.0, 2000.0)

counter = 0

while 1:
    dev.poll(50)
    counter += 1
    print(counter)
    sensor1.update(counter)
    if counter > 1999:
        counter = 0
