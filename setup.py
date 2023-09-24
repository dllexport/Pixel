import git
import sys
import os
import traceback
import shutil

from git import Repo
from git import RemoteProgress
from tqdm import tqdm

class CloneProgress(RemoteProgress):
    def __init__(self, name):
        super().__init__()
        self.pbar = tqdm()
        self.name = name

    def update(self, op_code, cur_count, max_count=None, message=''):
        self.pbar.desc = self.name
        self.pbar.total = max_count
        self.pbar.n = cur_count
        self.pbar.refresh()

def GitClone(path: str, url: str):
    # rm folder before
    if os.path.exists(path):
        shutil.rmtree(path)

    try:
        repo = git.Repo.clone_from(url, path, progress=CloneProgress(path))
        return repo
    except git.exc.GitCommandError:
        traceback.print_exc()
        sys.exit(-1)

def GitCheck(path: str, url: str, branch: str):
    repo = None
    try:
        repo = git.Repo(path)
    except (git.exc.NoSuchPathError, git.exc.InvalidGitRepositoryError):
        repo = GitClone(path, url)

    try:
        repo.remotes[0].fetch()
        repo.git.checkout(branch)
    except git.exc.GitCommandError:
        traceback.print_exc()
        sys.exit(-1)

GitCheck("3rd/glfw", "https://github.com/glfw/glfw.git", "3.3.8")
GitCheck("3rd/glm", "https://github.com/g-truc/glm.git", "0.9.9.8")
GitCheck("3rd/imgui", "https://github.com/ocornut/imgui.git", "v1.89.9")
GitCheck("3rd/json_struct", "https://github.com/jorgen/json_struct.git", "master")
GitCheck("3rd/KTX-Software", "https://github.com/KhronosGroup/KTX-Software.git", "v4.2.1")
GitCheck("3rd/spdlog", "https://github.com/gabime/spdlog.git", "v1.12.0")
GitCheck("3rd/SPIRV-Reflect", "https://github.com/KhronosGroup/SPIRV-Reflect.git", "sdk-1.3.261.1")
GitCheck("3rd/tinygltf", "https://github.com/syoyo/tinygltf.git", "v2.8.17")
GitCheck("3rd/VulkanMemoryAllocator", "https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git", "v3.0.1")
