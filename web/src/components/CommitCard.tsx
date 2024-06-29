/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import {
    faArrowsAlt,
    faCirclePlus,
    faCopy,
    faDeleteLeft,
    faTrashAlt,
    faTrashCan
} from "@fortawesome/free-solid-svg-icons";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { useMemo } from "react";
import { Badge, Button, Card, CardProps } from "react-daisyui";
import SectionLabel from "./SectionLabel";
import _ from "lodash";

export type CommitCardProps = CardProps & {
    section: Section;
    commit: Commit;
    onActivate?: (section: Section, commit: Commit) => void;
    onDeleteRequested?: (section: Section) => void;
};

export default function CommitCard(props: CommitCardProps) {
    const { branch, repository, architecture } = useMemo(
        () => props.section,
        [props.section]
    );

    const { toAdd, toDelete, toCopy, toMove } = useMemo(
        () => props.commit,
        [props.commit]
    );

    return (
        <Card
            onClick={() => props.onActivate?.(props.section, props.commit)}
            {...props}
            className="bg-white"
            compact="sm"
        >
            <Card.Body className="w-full flex-col">
                <div className="flex justify-center">
                    <Badge color="secondary" className="float-right text-white">
                        <SectionLabel
                            section={{ branch, repository, architecture }}
                        />
                    </Badge>
                </div>
                <div className="grid grid-cols-2 grid-rows-2 gap-2">
                    {[
                        {
                            visible: toAdd.size > 0,
                            icon: faCirclePlus,
                            text: `To Add: ${toAdd.size}`
                        },
                        {
                            visible: toDelete.size > 0,
                            icon: faTrashAlt,
                            text: `To Delete: ${toDelete.size}`
                        },
                        {
                            visible: toCopy.size > 0,
                            icon: faCopy,
                            text: `To Copy: ${toCopy.size}`
                        },
                        {
                            visible: toMove.size > 0,
                            icon: faArrowsAlt,
                            text: `To Move: ${toMove.size}`
                        }
                    ].map(
                        ({ visible, icon, text }, index) =>
                            visible && (
                                <span
                                    key={index}
                                    className="justify-self-center self-center"
                                >
                                    <FontAwesomeIcon
                                        icon={icon}
                                        className="mr-1"
                                    />
                                    {text}
                                </span>
                            )
                    )}
                </div>
            </Card.Body>
            <Card.Actions>
                <Button className="w-full flex justify-end" color="ghost">
                    <FontAwesomeIcon
                        onClick={(e) => {
                            e.stopPropagation();
                            if (props.onDeleteRequested)
                                return props.onDeleteRequested(props.section);
                        }}
                        className="px-1"
                        icon={faDeleteLeft}
                    />
                </Button>
            </Card.Actions>
        </Card>
    );
}
