import hid
import time
import threading
from settings import Settings

class Mouse:
    """
    Mouse 类现在负责管理与 HID 鼠标设备的连接，
    并直接通过 USB 发送移动和点击命令。

    Attributes:
        settings (Settings): 用于检索配置设置的 Settings 类的实例。
        lock (threading.Lock): 用于确保线程安全的锁。
        device (hid.device): 与 HID 设备的连接。
        remainder_x (float): x轴移动的累积余数，用于平滑移动。
        remainder_y (float): y轴移动的累积余数，用于平滑移动。
    """

    def __init__(self):
        """
        初始化 Mouse 类，设置并连接到 HID 设备。
        """
        self.settings = Settings()
        self.lock = threading.Lock()

        # 从 settings.ini 读取供应商ID (Vendor ID) 和产品ID (Product ID)
        # 假设它们以十六进制字符串形式存储 (例如 "0x1532")
        try:
            vendor_id_str = self.settings.get('HID', 'vendor_id')
            product_id_str = self.settings.get('HID', 'product_id')
            self.vendor_id = int(vendor_id_str, 16)
            self.product_id = int(product_id_str, 16)
        except Exception as e:
            print(f"从 settings.ini 读取 HID 配置时出错: {e}")
            print("请确保您有一个 [HID] 部分，并包含 vendor_id 和 product_id。")
            print("程序将在10秒后退出...")
            time.sleep(10)
            exit()

        self.device = hid.device()
        try:
            self.device.open(self.vendor_id, self.product_id)
            print("已成功连接到 HID 设备。")
        except Exception as e:
            print(f"连接 HID 设备失败: {e}")
            print("请检查 settings.ini 中的 VID/PID 是否正确，并确保设备已连接。")
            print("程序将在10秒后退出...")
            time.sleep(10)
            exit()

        self.remainder_x = 0.0
        self.remainder_y = 0.0

    def _unsign(self, value):
        """
        将值转换为8位无符号整数，以匹配 HID 报告中 int8_t 的类型。
        """
        return value & 0xFF

    def move(self, x, y):
        """
        向 HID 设备发送鼠标移动命令。
        通过处理小数移动的余数来平滑移动。

        Args:
            x (float): x轴上的移动距离。
            y (float): y轴上的移动距离。
        """
        with self.lock:
            # 加上次计算的余数
            x += self.remainder_x
            y += self.remainder_y

            # 对x和y取整，并计算新的余数
            move_x = int(x)
            move_y = int(y)
            self.remainder_x = x - move_x
            self.remainder_y = y - move_y

            if move_x != 0 or move_y != 0:
                # 构建 HID 报告 [buttons, x, y]
                # 0 代表没有按键按下
                report = [0, self._unsign(move_x), self._unsign(move_y)]
                try:
                    self.device.write(report)
                except Exception as e:
                    print(f"向 HID 设备写入时出错: {e}")


    def click(self):
        """
        向 HID 设备发送鼠标左键单击命令。
        """
        with self.lock:
            try:
                # 按下鼠标左键 (按钮1)
                # 报告格式: [按键状态, x移动, y移动]
                self.device.write([1, 0, 0])

                # 理论上需要一个短暂的延迟
                time.sleep(0.01)

                # 释放鼠标左键 (按钮0)
                self.device.write([0, 0, 0])
            except Exception as e:
                print(f"在点击期间向 HID 设备写入时出错: {e}")

    def close(self):
        """
        关闭 HID 设备连接。
        """
        if self.device:
            self.device.close()

    def __del__(self):
        """
        确保在对象销毁时关闭设备连接。
        """
        self.close()
