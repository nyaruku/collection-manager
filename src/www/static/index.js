'use strict';

let hideUnknownMaps = false;

const hiddenColumns = new Set();

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


function toggleHideUnknown() {
    hideUnknownMaps = !hideUnknownMaps;
    document.getElementById('hide-unknown-item').textContent =
        hideUnknownMaps ? 'Show Unknown Maps' : 'Hide Unknown Maps';
    document.querySelectorAll('.beatmap-row[data-unknown]').forEach(row => {
        row.classList.toggle('d-none', hideUnknownMaps);
    });
}

const columnDefs = [
    { name: 'Title',      col: 'title',      hideable: false },
    { name: 'Artist',     col: 'artist',     hideable: true },
    { name: 'Difficulty', col: 'difficulty', hideable: true },
    { name: 'Mapper',     col: 'mapper',     hideable: true },
    { name: 'Stars',      col: 'stars',      hideable: true },
    { name: 'ID',         col: 'id',         hideable: true },
    { name: 'Set',        col: 'setid',      hideable: true },
    { name: 'MD5',        col: 'md5',        hideable: true, extraClass: 'col-md5' },
];

function setColumnVisible(col, visible) {
    if (!visible) hiddenColumns.add(col); else hiddenColumns.delete(col);
    document.querySelectorAll(`[data-col="${col}"]`).forEach(el => el.classList.toggle('d-none', !visible));
}

function showColumnMenu(event) {
    event.preventDefault();
    const menu = document.getElementById('col-context-menu');
    menu.innerHTML = '';

    columnDefs.forEach(def => {
        const item = document.createElement('div');
        item.className = 'col-context-item' + (!def.hideable ? ' disabled' : '');

        const checkbox = document.createElement('input');
        checkbox.type = 'checkbox';
        checkbox.checked = !hiddenColumns.has(def.col);
        checkbox.disabled = !def.hideable;

        const label = document.createElement('span');
        label.textContent = def.name;

        item.appendChild(checkbox);
        item.appendChild(label);

        if (def.hideable) {
            item.addEventListener('click', () => {
                const visible = hiddenColumns.has(def.col);
                checkbox.checked = visible;
                setColumnVisible(def.col, visible);
            });
        }
        menu.appendChild(item);
    });

    menu.style.left = Math.min(event.clientX, window.innerWidth - 180) + 'px';
    menu.style.top = Math.min(event.clientY, window.innerHeight - menu.offsetHeight - 8) + 'px';
    menu.classList.remove('d-none');
}

document.addEventListener('click', () => {
    document.getElementById('col-context-menu').classList.add('d-none');
});


async function reloadAll() {
    setStatus('Reloading...');
    await fetch('/api/reload', { method: 'POST' });
    await Promise.all([loadCollections('stable'), loadCollections('lazer')]);
    setStatus('Ready');
}

async function recompileScss() {
    setStatus('Recompiling SCSS...');
    try {
        const response = await fetch('/api/recompile-scss', { method: 'POST' });
        const data = await response.json();
        if (data.status === 'ok') {
            setStatus('SCSS recompiled, reload the page to apply changes.');
        } else {
            setStatus('SCSS error: ' + (data.message || 'unknown error'));
        }
    } catch (error) {
        setStatus('SCSS error: ' + error.message);
    }
}

function sortList(listId, columnIndex) {
    const list = document.getElementById(listId);
    if (!list) return;

    const rows = Array.from(list.querySelectorAll('.beatmap-row'));

    if (!tableOrigOrder[listId]) {
        rows.forEach((row, index) => { row.dataset.origIndex = index; });
        tableOrigOrder[listId] = true;
    }

    const prevSort = tableSortState[listId] || { column: -1, direction: 0 };
    let direction;

    if (prevSort.column !== columnIndex) direction = 1;
    else if (prevSort.direction === 1) direction = -1;
    else if (prevSort.direction === -1) direction = 0;
    else direction = 1;

    tableSortState[listId] = { column: columnIndex, direction };

    list.querySelectorAll('.beatmap-header .beatmap-col').forEach((header, index) => {
        header.textContent = header.textContent.replace(/ [▲▼]$/, '');
        if (index === columnIndex && direction !== 0)
            header.textContent += direction === 1 ? ' ▲' : ' ▼';
    });

    if (direction === 0) {
        rows.slice()
            .sort((a, b) => Number(a.dataset.origIndex) - Number(b.dataset.origIndex))
            .forEach(row => list.appendChild(row));
        return;
    }

    const isStarsColumn = columnIndex === 4;
    rows.sort((a, b) => {
        const aCols = a.querySelectorAll('.beatmap-col');
        const bCols = b.querySelectorAll('.beatmap-col');
        const aText = aCols[columnIndex] ? aCols[columnIndex].textContent.trim() : '';
        const bText = bCols[columnIndex] ? bCols[columnIndex].textContent.trim() : '';
        return direction * (isStarsColumn
            ? (parseFloat(aText) || 0) - (parseFloat(bText) || 0)
            : aText.localeCompare(bText));
    });
    rows.forEach(row => list.appendChild(row));
}

function buildBeatmapList(collection, listId) {
    const list = cloneTemplate('tmpl-beatmap-list');
    list.id = listId;

    const header = cloneTemplate('tmpl-beatmap-header');
    columnDefs.forEach((def, index) => {
        const headerCol = cloneTemplate('tmpl-beatmap-header-col');
        headerCol.textContent = def.name;
        headerCol.dataset.col = def.col;
        if (def.extraClass) headerCol.classList.add(def.extraClass);
        if (hiddenColumns.has(def.col)) headerCol.classList.add('d-none');
        headerCol.addEventListener('click', () => sortList(listId, index));
        header.appendChild(headerCol);
    });
    list.appendChild(header);

    collection.beatmaps.forEach(beatmap => {
        const notDownloaded = !beatmap.title || beatmap.title === '(not downloaded)';
        const row = cloneTemplate('tmpl-beatmap-row');

        if (notDownloaded) {
            row.classList.add('text-muted', 'fst-italic');
            row.dataset.unknown = '1';
            if (hideUnknownMaps) row.classList.add('d-none');
        }

        columnDefs.forEach(def => {
            const cell = row.querySelector(`[data-col="${def.col}"]`);
            if (!cell) return;
            if (def.extraClass) cell.classList.add(def.extraClass);
            if (hiddenColumns.has(def.col)) cell.classList.add('d-none');

            if (def.col === 'title') {
                cell.textContent = notDownloaded ? 'not downloaded' : beatmap.title;
            } else if (def.col === 'artist') {
                cell.textContent = beatmap.artist;
            } else if (def.col === 'difficulty') {
                cell.textContent = beatmap.difficulty;
            } else if (def.col === 'mapper') {
                cell.textContent = beatmap.mapper;
            } else if (def.col === 'stars') {
                cell.textContent = formatStars(beatmap.stars);
            } else if (def.col === 'id' && beatmap.id > 0) {
                const link = document.createElement('a');
                link.href = 'https://osu.ppy.sh/b/' + beatmap.id;
                link.target = '_blank';
                link.textContent = beatmap.id;
                cell.appendChild(link);
            } else if (def.col === 'setid' && beatmap.setId > 0) {
                const link = document.createElement('a');
                link.href = 'https://osu.ppy.sh/beatmapsets/' + beatmap.setId;
                link.target = '_blank';
                link.textContent = beatmap.setId;
                cell.appendChild(link);
            } else if (def.col === 'md5') {
                cell.textContent = beatmap.md5;
            }
        });

        list.appendChild(row);
    });

    return list;
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

    const listId = 'lst-' + (++detailSequence);

    const header = cloneTemplate('tmpl-collection-header');
    header.querySelector('[data-slot="name"]').innerHTML = escapeHtmlKeepLeadingSpaces(collection.name);
    header.querySelector('[data-slot="count"]').textContent = collection.beatmaps.length + ' Maps';
    fragment.appendChild(header);
    fragment.appendChild(buildBeatmapList(collection, listId));

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
    document.getElementById('status-text').innerHTML = 'Loading "' + escapeHtmlKeepLeadingSpaces(name) + '"...';

    try {
        const response = await fetch('/api/' + mode + '/collections?name=' + encodeURIComponent(name));
        if (!response.ok) throw new Error(await response.text());
        const collection = await response.json();
        detailPanel.replaceChildren(renderBeatmaps(collection));
        document.getElementById('status-text').innerHTML =
            '"' + escapeHtmlKeepLeadingSpaces(name) + '" (' + (collection.beatmaps ? collection.beatmaps.length : 0) + ' beatmaps)';
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
