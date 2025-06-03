import subprocess
import signal

class ProcessController:
  def __init__(self, cmd: list[str], cwd = None):
    self.cmd = cmd
    self.cwd = cwd
    self.process = None

  def start(self):
    if self.process is not None:
      raise RuntimeError("Process already started")
    self.process = subprocess.Popen(self.cmd, cwd = self.cwd)
    self.process.wait()
    return self.process

  def stop(self, force: bool = False):
    try:
      if not self.is_running():
        return
      if force:
        self.process.kill()
        self.process.wait()
        return

      self.process.terminate()
      self.process.wait()
      return
    except Exception as e:
      raise e
    finally:
      self.process = None

  def is_running(self):
    return self.process and self.process.poll() is None
