<script lang="ts">
  import { onMount } from 'svelte';
  import { writable } from 'svelte/store';

  let files: { name: string; size: number }[] = [];
  let dragOver = false;
  const editingFile = writable<{ name: string; content: string } | null>(null);

  async function fetchFiles() {
    const res = await fetch(`/api/files`);
    files = await res.json();
  }

  async function handleDrop(event: DragEvent) {
    event.preventDefault();
    dragOver = false;

    const uploadFiles = event.dataTransfer?.files;
    if (!uploadFiles || uploadFiles.length === 0) return;

    for (const file of uploadFiles) {
      const content = await file.text();
      // send raw body
      const res = await fetch(`/api/file?file=${encodeURIComponent(file.name)}`, {
        method: 'POST',
        headers: { 'Content-Type': 'text/plain' },
        body: content
      });

      if (res.ok) fetchFiles();
    }
  }


  async function deleteFile(name: string) {
    if (!confirm(`Delete "${name}"?`)) return;
    await fetch(`/api/file`, {
      method: 'DELETE',
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
      body: `file=${encodeURIComponent(name)}`
    });
    fetchFiles();
  }

  async function editFile(name: string) {
    const res = await fetch(`/api/file?file=${encodeURIComponent(name)}`);
    if (!res.ok) return alert('Failed to read file');
    const content = await res.text();
    editingFile.set({ name, content });
  }

  async function saveFile(file: { name: string; content: string }) {
    const res = await fetch(`/api/file?file=${encodeURIComponent(file.name)}`, {
      method: 'POST',
      headers: { 'Content-Type': 'text/plain' },
      body: file.content
    });

    if (res.ok) {
      editingFile.set(null);
      fetchFiles();
    } else {
      alert('Failed to save file');
    }
  }

  onMount(fetchFiles);
</script>

<div
  class="border-4 border-dashed border-gray-400 rounded-lg p-12 text-center transition-colors"
  class:!bg-blue-100={dragOver}
  on:dragover|preventDefault={() => (dragOver = true)}
  on:dragleave={() => (dragOver = false)}
  on:drop={handleDrop}
>
  <p class="text-gray-600 font-semibold">{dragOver ? 'Release to upload' : 'Drag & Drop files here'}</p>
</div>

<table class="table-auto w-full mt-6 border-collapse">
  <thead>
    <tr class="bg-gray-200">
      <th class="px-4 py-2 text-left">Filename</th>
      <th class="px-4 py-2 text-right">Size</th>
      <th class="px-4 py-2 text-center">Actions</th>
    </tr>
  </thead>
  <tbody>
    {#each files as file}
      <tr class="border-b hover:bg-gray-50">
        <td class="px-4 py-2 cursor-pointer text-blue-600 hover:underline" on:click={() => editFile(file.name)}>
          {file.name}
        </td>
        <td class="px-4 py-2 text-right">{file.size} bytes</td>
        <td class="px-4 py-2 text-center">
          <button
            class="bg-red-500 hover:bg-red-700 text-white font-bold py-1 px-3 rounded shadow"
            on:click={() => deleteFile(file.name)}
          >
            Delete
          </button>
        </td>
      </tr>
    {/each}
  </tbody>
</table>

<!-- Modal -->
{#if $editingFile}
  <div class="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
    <div class="bg-white rounded-lg w-3/4 max-w-2xl p-6 shadow-lg">
      <h2 class="text-xl font-bold mb-4">Editing: {$editingFile.name}</h2>
      <textarea
        bind:value={$editingFile.content}
        class="w-full h-64 border border-gray-300 rounded p-2 font-mono"
      ></textarea>
      <div class="mt-4 flex justify-end space-x-2">
        <button class="bg-gray-300 hover:bg-gray-400 text-black font-bold py-1 px-4 rounded" on:click={() => editingFile.set(null)}>
          Cancel
        </button>
        <button class="bg-blue-500 hover:bg-blue-700 text-white font-bold py-1 px-4 rounded" on:click={() => saveFile($editingFile)}>
          Save
        </button>
      </div>
    </div>
  </div>
{/if}
