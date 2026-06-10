import sounddevice as sd

def callback(indata, outdata, frames, time, status):
    if status:
        print(status)
    outdata[:] = indata

print("Усилитель звука запущен.")
print("Нажмите Ctrl+C для остановки.")

try:
    with sd.Stream(
        channels=1,
        callback=callback,
        samplerate=44100
    ):
        input()
except KeyboardInterrupt:
    print("\nУсилитель остановлен.")
