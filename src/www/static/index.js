'use strict';

let showMD5Column = true;

const collectionCounts = {};
const tableSortState = {};
const tableOrigOrder = {};

let detailSequence = 0;

function setStatus(text) {
    document.getElementById('status-text').textContent = text;
}

function updateStatusCounts() {
    document.getElementById('status-counts').textContent =
        Object.entries(collectionCounts)
            .map(([mode, count]) => mode + ': ' + count + ' collections')
            .join('  ');
}

function showAbout() {
    const dialog = document.getElementById('about-dialog');
    dialog.style.transform = 'none';
    dialog.classList.remove('d-none');
    dialog.style.left = Math.max(0, (window.innerWidth - dialog.offsetWidth) / 2) + 'px';
    dialog.style.top = Math.max(0, (window.innerHeight - dialog.offsetHeight) / 3) + 'px';
}

(function initAboutDrag() {
    const dialog = document.getElementById('about-dialog');
    const titlebar = document.getElementById('about-titlebar');

    titlebar.addEventListener('mousedown', function(event) {
        event.preventDefault();

        // Snapshot rendered position before removing any CSS transform,
        // otherwise the dialog jumps on the first drag.
        const rect = dialog.getBoundingClientRect();
        dialog.style.transform = 'none';
        dialog.style.left = rect.left + 'px';
        dialog.style.top = rect.top + 'px';

        const originX = event.clientX - rect.left;
        const originY = event.clientY - rect.top;

        document.onmousemove = function(moveEvent) {
            const maxX = window.innerWidth - dialog.offsetWidth;
            const maxY = window.innerHeight - dialog.offsetHeight;
            dialog.style.left = Math.min(Math.max(0, moveEvent.clientX - originX), maxX) + 'px';
            dialog.style.top = Math.min(Math.max(0, moveEvent.clientY - originY), maxY) + 'px';
        };

        document.onmouseup = function() {
            document.onmousemove = null;
            document.onmouseup = null;
        };
    });
})();

function escapeHtml(str) {
    return (str || '')
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;');
}

// Replaces leading spaces with &nbsp; so browsers don't collapse them.
function escapeHtmlKeepLeadingSpaces(str) {
    return escapeHtml(str).replace(/^ +/, spaces => '&nbsp;'.repeat(spaces.length));
}

function formatStars(rating) {
    return rating > 0 ? rating.toFixed(2) + '\u2605' : '';
}

function switchTab(mode) {
    const button = document.getElementById('tab-' + mode);
    if (button) bootstrap.Tab.getOrCreateInstance(button).show();
}

function toggleMD5() {
    showMD5Column = !showMD5Column;
    document.getElementById('md5-toggle-item').textContent = showMD5Column ? 'Hide MD5' : 'Show MD5';
    document.querySelectorAll('.col-md5').forEach(cell => cell.classList.toggle('d-none', !showMD5Column));
}

async function reloadAll() {
    setStatus('Reloading...');
    await fetch('/api/reload', { method: 'POST' });
    await Promise.all([loadCollections('stable'), loadCollections('lazer')]);
    setStatus('Ready');
}

function sortTable(tableId, columnIndex) {
    const table = document.getElementById(tableId);
    if (!table) return;

    const tbody = table.querySelector('tbody');
    const rows = Array.from(tbody.querySelectorAll('tr'));

    if (!tableOrigOrder[tableId]) {
        rows.forEach((row, index) => { row.dataset.origIndex = index; });
        tableOrigOrder[tableId] = true;
    }

    const prevSort = tableSortState[tableId] || { column: -1, direction: 0 };
    let direction;

    if (prevSort.column !== columnIndex) direction = 1;
    else if (prevSort.direction === 1) direction = -1;
    else if (prevSort.direction === -1) direction = 0;
    else direction = 1;

    tableSortState[tableId] = { column: columnIndex, direction };

    table.querySelectorAll('thead th').forEach((header, index) => {
        header.textContent = header.textContent.replace(/ [▲▼]$/, '');
        if (index === columnIndex && direction !== 0)
            header.textContent += direction === 1 ? ' ▲' : ' ▼';
    });

    if (direction === 0) {
        rows.slice()
            .sort((a, b) => Number(a.dataset.origIndex) - Number(b.dataset.origIndex))
            .forEach(row => tbody.appendChild(row));
        return;
    }

    const isStarsColumn = columnIndex === 4;
    rows.sort((a, b) => {
        const aText = a.cells[columnIndex] ? a.cells[columnIndex].textContent.trim() : '';
        const bText = b.cells[columnIndex] ? b.cells[columnIndex].textContent.trim() : '';
        return direction * (isStarsColumn
            ? (parseFloat(aText) || 0) - (parseFloat(bText) || 0)
            : aText.localeCompare(bText));
    });
    rows.forEach(row => tbody.appendChild(row));
}

function buildBeatmapTable(collection, tableId, md5Class) {
    const columns = ['Title', 'Artist', 'Difficulty', 'Mapper', 'Stars', 'MD5'];

    const headers = columns.map((name, index) => {
        const extra = index === 5 ? ' ' + md5Class : '';
        return `<th class="py-0 px-1 bg-black sortable-header${extra}"
                    onclick="sortTable('${tableId}', ${index})">${name}</th>`;
    }).join('');

    const rows = collection.beatmaps.map(beatmap => {
        const notDownloaded = beatmap.title === '(not downloaded)';
        const rowClass = notDownloaded ? ' class="text-muted fst-italic"' : '';
        const titleCell = notDownloaded ? '<em>not downloaded</em>' : escapeHtml(beatmap.title);
        return `<tr${rowClass}>
            <td class="py-0 px-1">${titleCell}</td>
            <td class="py-0 px-1">${escapeHtml(beatmap.artist)}</td>
            <td class="py-0 px-1">${escapeHtml(beatmap.difficulty)}</td>
            <td class="py-0 px-1">${escapeHtml(beatmap.mapper)}</td>
            <td class="py-0 px-1 text-warning">${formatStars(beatmap.stars)}</td>
            <td class="py-0 px-1 font-monospace text-muted ${md5Class}">${beatmap.md5}</td>
        </tr>`;
    }).join('');

    return `<table id="${tableId}" class="table table-hover table-sm table-bordered mb-0">
        <thead class="sticky-top"><tr>${headers}</tr></thead>
        <tbody>${rows}</tbody>
    </table>`;
}

function buildBeatmapCards(collection) {
    return collection.beatmaps.map(beatmap => {
        const notDownloaded = beatmap.title === '(not downloaded)';
        return `<div class="list-group-item list-group-item-action p-2 border-0 border-bottom${notDownloaded ? ' text-muted' : ''}">
            <div class="${notDownloaded ? 'fst-italic' : 'fw-semibold'} text-truncate">${notDownloaded ? 'not downloaded' : escapeHtml(beatmap.title)}</div>
            <div class="text-muted">${escapeHtml(beatmap.artist)}</div>
            <div class="d-flex gap-3 mt-1">
                <span class="text-muted">${escapeHtml(beatmap.difficulty)}</span>
                <span class="text-muted">${escapeHtml(beatmap.mapper)}</span>
                <span class="text-warning ms-auto">${formatStars(beatmap.stars)}</span>
            </div>
        </div>`;
    }).join('');
}

function renderBeatmaps(collection) {
    const name = escapeHtmlKeepLeadingSpaces(collection.name);

    if (!collection.beatmaps || collection.beatmaps.length === 0)
        return `<p class="text-muted p-2 m-0">${name} - no beatmaps</p>`;

    const tableId = 'tbl-' + (++detailSequence);
    const md5Class = 'col-md5' + (showMD5Column ? '' : ' d-none');

    return `
        <div class="px-2 border-bottom bg-dark-subtle d-flex align-items-center">
            <span class="fw-semibold">${name}</span>
            <span class="text-muted ms-2">${collection.beatmaps.length} Maps</span>
        </div>
        <div class="d-none d-md-block">${buildBeatmapTable(collection, tableId, md5Class)}</div>
        <div class="d-md-none list-group list-group-flush">${buildBeatmapCards(collection)}</div>`;
}

async function loadCollections(mode) {
    const listElement = document.getElementById(mode + '-list');
    try {
        const response = await fetch('/api/' + mode + '/collections');
        if (!response.ok) throw new Error(await response.text());
        const collections = await response.json();

        const label = mode[0].toUpperCase() + mode.slice(1);
        collectionCounts[label] = collections.length;
        updateStatusCounts();

        if (collections.length === 0) {
            listElement.innerHTML = '<div class="list-group-item text-muted py-1 border-0 fst-italic">No collections</div>';
            return;
        }

        listElement.innerHTML = collections.map(collection => {
            const safeName = (collection.name || '').replace(/&/g, '&amp;').replace(/"/g, '&quot;');
            const displayName = escapeHtmlKeepLeadingSpaces(collection.name) || '<em class="text-muted">(unnamed)</em>';
            return `
                <button type="button"
                        class="list-group-item list-group-item-action py-1 px-2 border-0 rounded-0"
                        data-name="${safeName}"
                        onclick="selectCollection('${mode}', this)">
                    <div class="d-flex justify-content-between align-items-center gap-1">
                        <span class="text-truncate">${displayName}</span>
                        <span class="badge bg-white text-black fw-normal flex-shrink-0 small">${collection.count}</span>
                    </div>
                </button>
            `;
        }).join('');

    } catch (error) {
        listElement.innerHTML = `<div class="list-group-item text-danger py-1 border-0">${escapeHtml(error.message)}</div>`;
        setStatus('Error: ' + error.message);
    }
}

async function selectCollection(mode, element) {
    document.querySelectorAll('#' + mode + '-list .list-group-item')
        .forEach(item => item.classList.remove('active'));
    element.classList.add('active');

    const name = element.dataset.name;
    const detailPanel = document.getElementById(mode + '-detail');
    detailPanel.innerHTML = '<p class="text-muted p-2 m-0">Loading...</p>';
    setStatus('Loading "' + name + '"...');

    try {
        const response = await fetch('/api/' + mode + '/collections?name=' + encodeURIComponent(name));
        if (!response.ok) throw new Error(await response.text());
        const collection = await response.json();
        detailPanel.innerHTML = renderBeatmaps(collection);
        setStatus('"' + name + '" - ' + (collection.beatmaps ? collection.beatmaps.length : 0) + ' beatmaps');
    } catch (error) {
        detailPanel.innerHTML = `<div class="alert alert-danger m-2 py-1 rounded-0">${escapeHtml(error.message)}</div>`;
        setStatus('Error: ' + error.message);
    }
}

loadCollections('stable');
loadCollections('lazer');
