# 2026-03-27T13:35:47.579631
import vitis

client = vitis.create_client()
client.set_workspace(path="HLS")

vitis.dispose()

