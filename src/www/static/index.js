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

// Replaces leading spaces with &nbsp; so browsers don't collapse them.
function escapeHtmlKeepLeadingSpaces(str) {
    return (str || '')
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/^ +/, spaces => '&nbsp;'.repeat(spaces.length));
}

function formatStars(rating) {
    return rating > 0 ? rating.toFixed(2) + '\u2605' : '';
}

function cloneTemplate(id) {
    return document.getElementById(id).content.cloneNode(true).firstElementChild;
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

function buildBeatmapTable(collection, tableId) {
    const columns = ['Title', 'Artist', 'Difficulty', 'Mapper', 'Stars', 'MD5'];

    const table = cloneTemplate('tmpl-beatmap-table');
    table.id = tableId;

    const headerRow = table.querySelector('thead tr');
    columns.forEach((name, index) => {
        const th = cloneTemplate('tmpl-beatmap-th');
        th.textContent = name;
        if (index === 5) {
            th.classList.add('col-md5');
            if (!showMD5Column) th.classList.add('d-none');
        }
        th.addEventListener('click', () => sortTable(tableId, index));
        headerRow.appendChild(th);
    });

    const tbody = table.querySelector('tbody');
    collection.beatmaps.forEach(beatmap => {
        const row = cloneTemplate('tmpl-beatmap-row');
        const cells = row.querySelectorAll('td');
        const notDownloaded = beatmap.title === '(not downloaded)';

        if (notDownloaded) {
            row.classList.add('text-muted', 'fst-italic');
            cells[0].innerHTML = '<em>not downloaded</em>';
        } else {
            cells[0].textContent = beatmap.title;
        }
        cells[1].textContent = beatmap.artist;
        cells[2].textContent = beatmap.difficulty;
        cells[3].textContent = beatmap.mapper;
        cells[4].textContent = formatStars(beatmap.stars);
        cells[5].textContent = beatmap.md5;
        if (!showMD5Column) cells[5].classList.add('d-none');

        tbody.appendChild(row);
    });

    return table;
}

function buildBeatmapCards(collection) {
    const fragment = document.createDocumentFragment();

    collection.beatmaps.forEach(beatmap => {
        const card = cloneTemplate('tmpl-beatmap-card');
        const notDownloaded = beatmap.title === '(not downloaded)';
        const titleEl = card.querySelector('[data-slot="title"]');

        if (notDownloaded) {
            card.classList.add('text-muted');
            titleEl.classList.replace('fw-semibold', 'fst-italic');
            titleEl.textContent = 'not downloaded';
        } else {
            titleEl.textContent = beatmap.title;
        }
        card.querySelector('[data-slot="artist"]').textContent = beatmap.artist;
        card.querySelector('[data-slot="difficulty"]').textContent = beatmap.difficulty;
        card.querySelector('[data-slot="mapper"]').textContent = beatmap.mapper;
        card.querySelector('[data-slot="stars"]').textContent = formatStars(beatmap.stars);

        fragment.appendChild(card);
    });

    return fragment;
}

function renderBeatmaps(collection) {
    const fragment = document.createDocumentFragment();

    if (!collection.beatmaps || collection.beatmaps.length === 0) {
        const p = document.createElement('p');
        p.className = 'text-muted p-2 m-0';
        p.innerHTML = escapeHtmlKeepLeadingSpaces(collection.name) + ' - no beatmaps';
        fragment.appendChild(p);
        return fragment;
    }

    const tableId = 'tbl-' + (++detailSequence);

    const header = cloneTemplate('tmpl-collection-header');
    header.querySelector('[data-slot="name"]').innerHTML = escapeHtmlKeepLeadingSpaces(collection.name);
    header.querySelector('[data-slot="count"]').textContent = collection.beatmaps.length + ' Maps';
    fragment.appendChild(header);

    const tableWrapper = document.createElement('div');
    tableWrapper.className = 'd-none d-md-block';
    tableWrapper.appendChild(buildBeatmapTable(collection, tableId));
    fragment.appendChild(tableWrapper);

    const cardsWrapper = document.createElement('div');
    cardsWrapper.className = 'd-md-none list-group list-group-flush';
    cardsWrapper.appendChild(buildBeatmapCards(collection));
    fragment.appendChild(cardsWrapper);

    return fragment;
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
            const div = document.createElement('div');
            div.className = 'list-group-item text-muted py-1 border-0 fst-italic';
            div.textContent = 'No collections';
            listElement.replaceChildren(div);
            return;
        }

        const fragment = document.createDocumentFragment();
        collections.forEach(collection => {
            const item = cloneTemplate('tmpl-collection-item');
            item.dataset.name = collection.name || '';
            item.querySelector('[data-slot="name"]').innerHTML =
                escapeHtmlKeepLeadingSpaces(collection.name) || '<em class="text-muted">(unnamed)</em>';
            item.querySelector('[data-slot="count"]').textContent = collection.count;
            item.addEventListener('click', () => selectCollection(mode, item));
            fragment.appendChild(item);
        });
        listElement.replaceChildren(fragment);

    } catch (error) {
        const div = document.createElement('div');
        div.className = 'list-group-item text-danger py-1 border-0';
        div.textContent = error.message;
        listElement.replaceChildren(div);
        setStatus('Error: ' + error.message);
    }
}

async function selectCollection(mode, element) {
    document.querySelectorAll('#' + mode + '-list .list-group-item')
        .forEach(item => item.classList.remove('active'));
    element.classList.add('active');

    const name = element.dataset.name;
    const detailPanel = document.getElementById(mode + '-detail');
    const loading = document.createElement('p');
    loading.className = 'text-muted p-2 m-0';
    loading.textContent = 'Loading...';
    detailPanel.replaceChildren(loading);
    setStatus('Loading "' + name + '"...');

    try {
        const response = await fetch('/api/' + mode + '/collections?name=' + encodeURIComponent(name));
        if (!response.ok) throw new Error(await response.text());
        const collection = await response.json();
        detailPanel.replaceChildren(renderBeatmaps(collection));
        setStatus('"' + name + '" - ' + (collection.beatmaps ? collection.beatmaps.length : 0) + ' beatmaps');
    } catch (error) {
        const div = document.createElement('div');
        div.className = 'alert alert-danger m-2 py-1 rounded-0';
        div.textContent = error.message;
        detailPanel.replaceChildren(div);
        setStatus('Error: ' + error.message);
    }
}

loadCollections('stable');
loadCollections('lazer');
