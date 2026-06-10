import sounddevice as sd
import tkinter as tk

stream = None

def callback(indata, outdata, frames, time, status):
    outdata[:] = indata

def start():
    global stream
    if stream is None:
        stream = sd.Stream(
            channels=1,
            callback=callback,
            samplerate=44100
        )
        stream.start()
        status_label.config(text="Усилитель включён")

def stop():
    global stream
    if stream:
        stream.stop()
        stream.close()
        stream = None
        status_label.config(text="Усилитель выключен")

root = tk.Tk()
root.title("Mic Amplifier")

tk.Button(root, text="Включить", command=start).pack(padx=10, pady=5)
tk.Button(root, text="Выключить", command=stop).pack(padx=10, pady=5)

status_label = tk.Label(root, text="Усилитель выключен")
status_label.pack(pady=10)

root.mainloop()
