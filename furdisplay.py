import cv2
import pygame
import sys

# -------------------------
# Pygame
# -------------------------
pygame.init()

SCREEN_W = 1000
SCREEN_H = 400

screen = pygame.display.set_mode((SCREEN_W, SCREEN_H))
pygame.display.set_caption("Robot Eyes Tracker")

# Загрузка глаз
left_eye_img = pygame.image.load("Eyes.jpeg").convert_alpha()
right_eye_img = pygame.image.load("Eyes2.jpeg").convert_alpha()

left_eye_img = pygame.transform.scale(left_eye_img, (200, 200))
right_eye_img = pygame.transform.scale(right_eye_img, (200, 200))

# -------------------------
# Camera
# -------------------------
cap = cv2.VideoCapture(0)

face_cascade = cv2.CascadeClassifier(
    cv2.data.haarcascades +
    "haarcascade_frontalface_default.xml"
)

# Положение глаз
eye_offset_x = 0
eye_offset_y = 0

clock = pygame.time.Clock()

# -------------------------
# Main loop
# -------------------------
running = True

while running:

    # pygame events
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    ret, frame = cap.read()

    if ret:

        frame = cv2.flip(frame, 1)

        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

        faces = face_cascade.detectMultiScale(
            gray,
            scaleFactor=1.3,
            minNeighbors=5
        )

        h, w = frame.shape[:2]

        # Центр камеры
        center_x = w // 2
        center_y = h // 2

        if len(faces) > 0:

            x, y, fw, fh = faces[0]

            face_x = x + fw // 2
            face_y = y + fh // 2

            # Рисуем рамку
            cv2.rectangle(
                frame,
                (x, y),
                (x + fw, y + fh),
                (0, 255, 0),
                2
            )

            # Движение глаз
            eye_offset_x = int((face_x - center_x) * 0.08)
            eye_offset_y = int((face_y - center_y) * 0.08)

            eye_offset_x = max(-25, min(25, eye_offset_x))
            eye_offset_y = max(-25, min(25, eye_offset_y))

        # OpenCV -> Pygame
        frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

        cam_surface = pygame.surfarray.make_surface(
            frame_rgb.swapaxes(0, 1)
        )

        cam_surface = pygame.transform.scale(
            cam_surface,
            (400, 300)
        )

        # -------------------------
        # Draw
        # -------------------------
        screen.fill((30, 30, 30))

        # Камера
        screen.blit(cam_surface, (10, 50))

        # Левый глаз
        screen.blit(
            left_eye_img,
            (
                500 + eye_offset_x,
                100 + eye_offset_y
            )
        )

        # Правый глаз
        screen.blit(
            right_eye_img,
            (
                750 + eye_offset_x,
                100 + eye_offset_y
            )
        )

        pygame.display.flip()

    clock.tick(60)

# -------------------------
# Cleanup
# -------------------------
cap.release()
cv2.destroyAllWindows()
pygame.quit()
sys.exit()
