import cv2
import pygame
import threading
import time
import random


# ------------------ EyeManager ------------------
class EyeManager:
    def __init__(self):
        pygame.init()
        self.screen = pygame.display.set_mode((320, 240))
        pygame.display.set_caption("FurDisplay Eyes")

        self.eye1 = [160, 120]
        self.eye2 = [160, 120]

        self.lock = threading.Lock()

    def update(self, coords):
        with self.lock:
            if coords:
                self.eye1 = list(coords[0])
                if len(coords) > 1:
                    self.eye2 = list(coords[1])
            else:
                self.eye1 = [160, 120]
                self.eye2 = [160, 120]

    def render(self):
        with self.lock:
            self.screen.fill((255, 255, 255))

            pygame.draw.rect(
                self.screen,
                (0, 0, 0),
                pygame.Rect(self.eye1[0] - 10, self.eye1[1] - 10, 20, 20),
            )

            pygame.draw.rect(
                self.screen,
                (0, 0, 0),
                pygame.Rect(self.eye2[0] - 10, self.eye2[1] - 10, 20, 20),
            )

            pygame.display.flip()


# ------------------ CameraManager (REAL OpenCV) ------------------
class CameraManager:
    def __init__(self):
        self.cap = cv2.VideoCapture(0)
        self.coords = []
        self.lock = threading.Lock()

        # Face detector (Haar cascade)
        self.face_cascade = cv2.CascadeClassifier(
            cv2.data.haarcascades + "haarcascade_frontalface_default.xml"
        )

    def capture_frames(self):
        ret, frame = self.cap.read()
        if not ret:
            return

        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

        faces = self.face_cascade.detectMultiScale(gray, 1.3, 5)

        coords = []

        for (x, y, w, h) in faces:
            cx = x + w // 2
            cy = y + h // 2
            coords.append((cx, cy))

        # если нет лица — случайное движение
        if not coords:
            h, w = frame.shape[:2]
            coords = [
                (random.randint(0, w), random.randint(0, h)),
                (random.randint(0, w), random.randint(0, h)),
            ]

        with self.lock:
            self.coords = coords

    def get_object_coordinates(self):
        with self.lock:
            return self.coords.copy()


# ------------------ BluetoothManager (stub) ------------------
class BluetoothManager:
    def send_status(self, coords, battery):
        print(f"[BT] battery={battery} coords={coords}")


# ------------------ BatteryManager ------------------
class BatteryManager:
    def get_status(self):
        return "95%"


# ------------------ MAIN ------------------
def main():
    eyes = EyeManager()
    camera = CameraManager()
    bt = BluetoothManager()
    battery = BatteryManager()

    running = True

    # --- Camera thread ---
    def cam_loop():
        while running:
            camera.capture_frames()
            time.sleep(0.03)

    # --- Eye render thread ---
    def eye_loop():
        while running:
            coords = camera.get_object_coordinates()
            eyes.update(coords)
            eyes.render()
            time.sleep(0.016)

    # --- Bluetooth thread ---
    def bt_loop():
        while running:
            coords = camera.get_object_coordinates()
            batt = battery.get_status()
            bt.send_status(coords, batt)
            time.sleep(0.5)

    threading.Thread(target=cam_loop, daemon=True).start()
    threading.Thread(target=eye_loop, daemon=True).start()
    threading.Thread(target=bt_loop, daemon=True).start()

    # main loop (pygame events)
    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
                return

        time.sleep(0.01)


if __name__ == "__main__":
    main()
