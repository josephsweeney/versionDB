import pygit2
import json
import os
import math
import datetime


class VersionDB(object):

    def __init__(self, repo_path, remote_url=None, credentials=None, init=False, logger=None):
        self.path = repo_path
        self.remote = remote_url is not None or credentials is not None
        self.logger = None if logger is None else logger
        if credentials is not None:
            username = credentials['username']
            pubkey = credentials['pubkey']
            privkey = credentials['privkey']
            passphrase = credentials['passphrase']
            self.credentials = pygit2.Keypair(
                username, pubkey, privkey, passphrase)
            self.callback = Callback(credentials=self.credentials)
        if remote_url is not None:
            pygit2.clone_repository(
                remote_url, repo_path, callbacks=self.callback)
            self.repo = pygit2.Repository(repo_path)
        else:
            if init:
                self.repo = pygit2.init_repository(repo_path)
            else:
                self.repo = pygit2.Repository(repo_path)

    def set_repo(self, repo_path, init=False):
        self.path = repo_path
        if not init:
            self.repo = pygit2.Repository(repo_path)
        else:
            self.repo = pygit2.init_repository(repo_path)

    def get(self, name, date_time=None):
        """Retrieves data with a given name at a given date_time"""
        repo = self.repo
        ref_name = 'refs/heads/' + name
        if self.remote:
            self.pull(ref_name)
        if date_time is None:
            remote_name = 'refs/remotes/origin/' + name
            if ref_name in repo.references:
                ref = repo.lookup_reference(ref_name)
            elif remote_name in repo.references:
                ref = repo.lookup_reference(remote_name)
            else:
                if self.logger:
                    msg = 'Could not find ref: {0}'.format(ref_name)
                    self.logger.error(msg)
                return None
            oid = ref.target
            commit = repo.get(oid)
            data = self._get_data_from_commit(name, commit)
            return data
        else:
            return self.get_at_time(name, date_time)

    def _split_path(self, path):
        """This splits paths like 'a/b/c/d' (can't have trailing slash) into [a, b, c, d]"""
        if path == None or path == '':
            return []
        result = []
        head, tail = os.path.split(path)
        while head and tail:
            result = [tail] + result
            path = head
            head, tail = os.path.split(path)
        result = [tail] + result
        return result

    def write(self, name, data, message=''):
        """
        Writes and commits data to file name.json.
        """
        if self.remote:
            self.pull(name)
        data_str = ''
        if data is not None:
            data_str = json.dumps(data, sort_keys=True, indent=4)
        head, file_str = os.path.split(name)
        if head is not None:
            path = os.path.join(self.path, head)
            if not os.path.exists(path):
                os.makedirs(path)

        path = os.path.join(self.path, name + '.json')
        # Write to that file
        with open(path, 'w+') as data_file:
            data_file.truncate()
            data_file.write(data_str)
        # Now commit our change
        return self._commit(message, name)

    def _commit(self, message, name):
        """Commit any changes to a specific file"""
        path = self.path
        repo = self.repo
        index = repo.index
        paths = [e.path for e in index]
        [index.remove(p) for p in paths]
        file_name = name + '.json'
        index.add(file_name)
        index.write()
        author = repo.default_signature
        commiter = repo.default_signature
        ref_name = 'refs/heads/' + name
        has_parent = True
        if ref_name in repo.references:
            ref = repo.lookup_reference(ref_name)
        else:
            has_parent = False
        tree = index.write_tree()
        parent = [] if not has_parent else [ref.target]
        oid = repo.create_commit(
            ref_name, author, commiter, message, tree, parent)
        if self.remote:
            pushed = self.push(name)
            while not pushed:
                if not self.pull(name):
                    return None
                ref = repo.lookup_reference(ref_name)
                oid = repo.create_commit(
                    ref_name, author, commiter, message, tree, [ref.target])
                pushed = self.push(name)
        return oid

    def pull(self, branch='master', remote_name='origin'):
        """
        This should pull down and integrate any changes from a remote repo.
        Returns True is succesful.
        """
        repo = self.repo
        remote = repo.remotes[remote_name]
        remote.fetch(callbacks=self.callback)
        remote_ref = 'refs/remotes/{0}/{1}'.format(remote_name, branch)
        local_ref = 'refs/heads/{0}'.format(branch)
        if remote_ref not in repo.references:
            if self.logger:
                msg = 'remote_ref {0} does not exits.'.format(remote_ref)
                logger.error(msg)
            return False
        remote_id = repo.lookup_reference(remote_ref).target
        if local_ref in repo.references:
            local_id = repo.lookup_reference(local_ref).target
            if local_id == remote_id:
                return True
            else:
                repo.lookup_reference(local_ref).set_target(remote_id)
                return True
        else:
            repo.references.create(local_ref, remote_id)
            return True

    def push(self, branch='master', remote_name='origin'):
        repo = self.repo
        try:
            remote = repo.remotes[remote_name]
            remote.push(['refs/heads/' + branch], callbacks=self.callback)
            return True
        except pygit2.GitError as err:
            msg = 'Could not push: {0}'.format(err)
            print(msg)
            if self.logger:
                logger.error(msg)
            return False

    def get_at_time(self, name, date_time):
        """Gets the venue config at a datetime"""
        repo = self.repo
        timestamp = date_time.timestamp()
        # Find the first commit that happened before that time
        past_commit = None
        ref_name = 'refs/heads/{0}'.format(name)
        try:
            oid = repo.lookup_reference(ref_name).target
        except KeyError:
            if self.logger:
                msg = 'Could not find ref_name {0}'.format(ref_name)
                self.logger.error(msg)
            return None
        for commit in repo.walk(oid, pygit2.GIT_SORT_NONE):
            commit_date_time = datetime.datetime.utcfromtimestamp(
                commit.commit_time)
            commit_time = commit_date_time.timestamp()
            if commit_time <= timestamp:
                past_commit = commit
                break
        # Now get the venue at that commit
        data_file = '{0}.json'.format(name)
        for entry in past_commit.tree:
            if entry.name == data_file:
                blob = repo.get(entry.id)
                return json.loads(blob.data)
        return None

    def _message(self, name, change_type='Unspecified'):
        """Create a commit message"""
        msg_dict = {'name': name, 'change_type': change_type}
        return json.dumps(msg_dict, sort_keys=True)

    def _get_data_from_commit(self, name, commit):
        """Get the data from a specific commit"""
        tree_type = pygit2.GIT_OBJ_TREE
        head, data_file = os.path.split(name)
        path = self._split_path(head)
        length = len(path)
        tree = commit.tree
        if length > 0:
            for d in path:
                tree_list = [self.repo.get(x.id) for x in tree if x.name == d]
                tree_list = [x for x in tree_list if x.type == tree_type]
                if len(tree_list) == 0:
                    return None
                else:
                    tree = tree_list[0]
        file_name = '{0}.json'.format(data_file)
        entry = list(filter(lambda x: x.name == file_name, tree))[0]
        try:
            data = json.loads(self.repo.get(entry.id).data)
        except json.decoder.JSONDecodeError as err:
            return None
        return data

    def _get_commit_history(self, name, start_stamp, end_stamp):
        """Gets the commit history for some data"""
        repo = self.repo
        commits = []
        ref_name = 'refs/heads/{0}'.format(name)
        oid = repo.lookup_reference(ref_name).target

        for commit in repo.walk(oid, pygit2.GIT_SORT_NONE):
            commit_date_time = datetime.datetime.utcfromtimestamp(
                commit.commit_time)
            commit_time = int(commit_date_time.timestamp())
            if commit_time <= end_stamp and commit_time >= start_stamp:
                commits.append(commit)
            elif commit_time < start_stamp:
                break
        return commits

    def _get_history_obj(self, commit, name):
        """Returns the history object associated with that commit"""
        return [commit.message]

    def get_data_history(self, name, start_time=None, end_time=None):
        """Get the history for some data"""
        repo = self.repo
        start_stamp = start_time.timestamp() if start_time else 0
        end_stamp = end_time.timestamp() if end_time else math.inf
        commits = self._get_commit_history(name, start_stamp, end_stamp)
        if commits is None:
            if self.logger:
                msg = 'No commit history for {0}.'.format(name)
                logger.error(msg)
            return None
        history = []
        for commit in commits:
            changes = self._get_history_obj(commit, name)
            history = history + changes
        return history

    def delete_data(self, name):
        """Deletes data with name 'name'"""
        if self.get(name) is None:
            return False
        msg = self._message(name, change_type='delete')
        self.write(name, None, message=msg)
        return True

    def add_data(self, name, data):
        """Adds data with name"""
        if self.get(name) is None:
            msg = self._message(name, 'add')
            self.write(name, data, message=msg)
            return data

    def replace_data(self, name, data):
        """Replace the entry name with data"""
        if self.get(name) is None:
            return False
        msg = self._message(name, 'update')
        self.write(name, data, message=msg)


class Callback(pygit2.RemoteCallbacks):
    pass
