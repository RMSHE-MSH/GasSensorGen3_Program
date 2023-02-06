import requests
from time import *
import random
import string


def generate_random_string(length: int) -> str:
    return ''.join(random.choices(string.digits, k=length))


url = "http://192.168.31.175/upload"

resp = requests.post(url)
Resp = resp.text
print(resp)
print(Resp)
resp.close()
